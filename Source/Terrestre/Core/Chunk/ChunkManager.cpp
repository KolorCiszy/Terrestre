// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Chunk.h"
#include "ChunkUtilityLib.h"
#include "Kismet/GameplayStatics.h"
#include "Terrestre/Core/Block/BlockData.h"
#include "Terrestre/Core/Character/Player/PlayerCharacter.h"
#include "Terrestre/Core/Gamemode/TerrestrePlayerState.h"
#include "Terrestre/Core/Gamemode/TerrestreGameModeBase.h"
#include "Terrestre/Core/Data Generators/TerrainShaper.h"
#include "Terrestre/Core/Data Generators/TerrainSurfaceDecorator.h"





AChunkManager::AChunkManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;
	RegionLoadDistance = 1;
	SetActorTickEnabled(false);
}

void AChunkManager::RegisterPlayerCharacter(TObjectPtr<APlayerCharacter> player)
{
	player->OnPlayerLocationChanged.AddDynamic(this, &AChunkManager::OnPlayerLocationChanged);
}

void AChunkManager::UnRegisterPlayerCharacter(TObjectPtr<APlayerCharacter> player) 
{
	player->OnPlayerLocationChanged.RemoveDynamic(this, &AChunkManager::OnPlayerLocationChanged);
}

void AChunkManager::GenerateStartingLocation()
{
	float cont = TerrainShaper->GetContinentalnessAtXY_Native(0, 0);
	float ero = TerrainShaper->GetErosionAtXY_Native(0, 0);
	float pv = TerrainShaper->GetPVAtXY_Native(0, 0);
	double terrainHeight = TerrainShaper->GetTerrainHeightAtXY_Native(0, 0, cont, ero, pv);

	DefaultPlayerSpawnLocation = FVector{ 0,0, (terrainHeight + 2) * 100 };
	ATerrestrePlayerState* playerState{};
	playerState = Cast<ATerrestrePlayerState>(UGameplayStatics::GetPlayerState(this, 0));
	if(playerState)
	{
		currentChunkLocation = UChunkUtilityLib::WorldLocationToChunkLocation(playerState->GetSpawnLocation());
	}
	else
	{
		currentChunkLocation = UChunkUtilityLib::WorldLocationToChunkLocation(DefaultPlayerSpawnLocation);
	}
}


void AChunkManager::CreateChunkMeshingThreadPool()
{
	int32 threadsToCreate = FPlatformMisc::NumberOfCoresIncludingHyperthreads() / 2;
	ChunkMeshingTP = FQueuedThreadPool::Allocate();
	if (threadsToCreate <= 0)
	{
		return;
	}
	if (ChunkMeshingTP)
	{
		ChunkMeshingTP->Create(threadsToCreate, 64 * 1024, EThreadPriority::TPri_Highest, TEXT("Chunk Meshing Thread Pool"));
	}
}

void AChunkManager::SpawnChunkActorPool()
{
	for (auto& location : SpawnedChunksLocations)
	{
		SpawnChunkAtLocation(location);
	}
}

void AChunkManager::BeginPlay()
{
	Super::BeginPlay();

	CreateChunkMeshingThreadPool();

	BlockData::Initialize(); 

	UChunkUtilityLib::ChunkManager = this;

	TerrainShaper = NewObject<UTerrainShaper>();
	TerrainShaper->Initialize();
	UChunkUtilityLib::TerrainShaper = TerrainShaper;
	TerrainSurfaceDecorator = NewObject<UTerrainSurfaceDecorator>();
	UChunkUtilityLib::TerrainSurfaceDecorator = TerrainSurfaceDecorator;

	GenerateStartingLocation();

	RegionManager = NewObject<URegionManager>();
	RegionManager->BeginPlay(currentChunkLocation);
	
	GenerateSpawnLocations(currentChunkLocation);
	
	while(!RegionManager->RegionsCurrentlyGenerating.IsEmpty())
	{
		RegionManager->Tick(currentChunkLocation);
		FPlatformProcess::Sleep(0.1f);
	}

	SpawnChunkActorPool();

	bShouldRecalculateActiveChunks = false;

	OnRebuildChunkMeshes.Broadcast();
	
	while(ActiveMeshingTasksCount > 0)
	{
		FPlatformProcess::Sleep(0.5f);
	}
	
	OnApplyChunkMeshes.Broadcast();
	
	auto gameMode = Cast<ATerrestreGameModeBase>(UGameplayStatics::GetGameMode(this));
	gameMode->SpawnPlayerCharacter(UGameplayStatics::GetPlayerState(this, 0));
	
	SetActorTickEnabled(true);
}

void AChunkManager::OnPlayerLocationChanged_Implementation(FVector currentLocation)
{
	//* Location of chunk the player is currently in
	FVector chunkLocation = UChunkUtilityLib::WorldLocationToChunkLocation(currentLocation);
	
	//* If it hasn't changed since the last update, we do nothing
	if (currentChunkLocation.Equals(chunkLocation, 20))
	{
		return;
	}
	currentChunkLocation = chunkLocation;
	//* If it had, generate new spawned chunks locations
	GenerateSpawnLocations(chunkLocation);
}

FBlockState AChunkManager::GetBlockAtWorldLocation(FVector worldLocation)
{
	FVector chunkLocation = UChunkUtilityLib::WorldLocationToChunkLocation(worldLocation);
	FChunkRegion* region = RegionManager->GetChunkRegionByID(UChunkUtilityLib::GetRegionID(chunkLocation));
	if(region)
	{
		FIntVector localBlockPos = UChunkUtilityLib::WorldLocationToLocalBlockPos(worldLocation);
		return region->ChunkData[chunkLocation].BlockPalette.GetBlockAtIndex(UChunkUtilityLib::LocalBlockPosToIndex(localBlockPos));
	}
	return FBlockState();
}

FFluidState AChunkManager::GetFluidAtWorldLocation(FVector worldLocation)
{
	FVector chunkLocation = UChunkUtilityLib::WorldLocationToChunkLocation(worldLocation);
	FChunkRegion* region = RegionManager->GetChunkRegionByID(UChunkUtilityLib::GetRegionID(chunkLocation));
	if (region)
	{
		FIntVector localBlockPos = UChunkUtilityLib::WorldLocationToLocalBlockPos(worldLocation);
		return region->ChunkData[chunkLocation].FluidStates[UChunkUtilityLib::LocalBlockPosToIndex(localBlockPos)];
	}
	return FFluidState();

}

FBlockPalette* AChunkManager::GetChunkBlockPalette(FVector chunkLocation)
{
	FChunkRegion* region = RegionManager->GetChunkRegionByID(UChunkUtilityLib::GetRegionID(chunkLocation));
	if(region)
	{
		return &region->ChunkData[chunkLocation].BlockPalette;
	}
	return nullptr;	
}

 bool AChunkManager::GetChunkFluidStates(FVector chunkLocation, TArray<FFluidState, TInlineAllocator<AChunk::Volume>>& fluidStates)
{
	FChunkRegion* region = RegionManager->GetChunkRegionByID(UChunkUtilityLib::GetRegionID(chunkLocation));
	if (region)
	{
		fluidStates = region->ChunkData[chunkLocation].FluidStates;
		return true;
	}
	return false;
}

bool AChunkManager::BulkUnpackChunkBlocks(FVector chunkLocation, TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& output)
{
	FChunkRegion*  region = RegionManager->GetChunkRegionByID(UChunkUtilityLib::GetRegionID(chunkLocation));
	if (region)
	{
		region->ChunkData[chunkLocation].BlockPalette.BulkUnpack(output);
		return true;
	}
	else
	{
		return false;
	}
}

void AChunkManager::GenerateSpawnLocations(FVector playerCurrentChunk)
{
	SpawnedChunksLocations.Empty(FMath::Pow((2 * RenderDistance) + 1, 3.0));
	
	for (int32 x = -RenderDistance; x <= RenderDistance; x++)
	{
		for (int32 y = -RenderDistance; y <= RenderDistance; y++)
		{
			for (int32 z = -RenderDistance; z <= RenderDistance; z++) 
			{
				FVector chunkWorldLocation{ x * AChunk::SizeScaled.X + playerCurrentChunk.X,
											y * AChunk::SizeScaled.Y + playerCurrentChunk.Y, 
											z * AChunk::SizeScaled.Z + playerCurrentChunk.Z };

				SpawnedChunksLocations.Add(chunkWorldLocation);
			}
		}
	}
	bShouldRecalculateActiveChunks = true;
}

void AChunkManager::RecalculateActiveChunks()
{
	LocationsToSpawn.Empty(FMath::Pow(RenderDistance, 3.0));
	LocationsToDespawn.Empty(FMath::Pow(RenderDistance, 3.0));
	for (auto& location : SpawnedChunksLocations)
	{
		if (!SpawnedChunksMap.Contains(location))
		{
			LocationsToSpawn.Add(location);
		}
	}
	for (auto& mapEntry : SpawnedChunksMap)
	{
		if (!SpawnedChunksLocations.Contains(mapEntry.Key))
		{
			LocationsToDespawn.Add(mapEntry.Key);
		}
	}
	bShouldRecalculateActiveChunks = false;
}


void AChunkManager::SetRenderDistance(uint8 newDistance)
{
	RenderDistance = FMath::Clamp(newDistance, MinRenderDistance, MaxRenderDistance);
	GenerateSpawnLocations(currentChunkLocation);
}

void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	OnApplyChunkMeshes.Broadcast();
	OnRebuildChunkMeshes.Broadcast();
	
	if (bShouldRecalculateActiveChunks)
	{
		RecalculateActiveChunks();
	}

	RegionManager->Tick(currentChunkLocation);
	

	TSet<FVector> removedChunkDespawnLocations{};
	int32 movedChunks = 0;
	for (auto& oldLocation : LocationsToDespawn)
	{
		const FVector& newLocation = *LocationsToSpawn.CreateConstIterator();
		FIntVector newRegionID = UChunkUtilityLib::GetRegionID(newLocation);
		FIntVector oldRegionID = UChunkUtilityLib::GetRegionID(oldLocation);
		FChunkRegion* newRegion = RegionManager->GetChunkRegionByID(newRegionID);
		FChunkRegion* oldRegion = RegionManager->GetChunkRegionByID(oldRegionID);
		
  		
		TObjectPtr<AChunk> chunk;
		chunk = SpawnedChunksMap[oldLocation];
		if (newRegion && chunk->bMeshingTaskDone)
		{
			chunk->ResetMesh();
			chunk->SetActorLocation(newLocation);
			chunk->MarkMeshDirty();
			SpawnedChunksMap.Add(newLocation, chunk);
			SpawnedChunksMap.Remove(oldLocation);

			if (newRegionID == oldRegionID)
			{
				newRegion->ChunkRefCount++;
			}
			else
			{
				newRegion->ChunkRefCount++;
				check(oldRegion->ChunkRefCount >= 0)
				oldRegion->ChunkRefCount--;
			}
			LocationsToSpawn.Remove(newLocation);
			removedChunkDespawnLocations.Add(oldLocation);
		}
		
		movedChunks++;
		if (movedChunks >= ChunksToMovePerTick)
		{
			break;
		}
	}

	for (auto& location : removedChunkDespawnLocations)
	{
		LocationsToDespawn.Remove(location);
	}
	
	
}
TObjectPtr<AChunk> AChunkManager::SpawnChunkAtLocation(const FVector inLocation)
{
	FTransform spawnTransform{inLocation};
	TObjectPtr<AChunk> chunk(GetWorld()->SpawnActor<AChunk>(ChunkClass, spawnTransform));
	FChunkRegion* region = RegionManager->GetChunkRegionByID(UChunkUtilityLib::GetRegionID(inLocation));
	SpawnedChunksMap.Add(inLocation, chunk);
	region->ChunkRefCount++;
	return chunk;
}
void AChunkManager::DestroyChunkAtLocation(const FVector inLocation)
{
	if(TObjectPtr<AChunk>* chunk = SpawnedChunksMap.Find(inLocation))
	{
		if(chunk->Get()->bReadyToDestroy)
		{
			chunk->Get()->Destroy();
			LocationsToDespawn.Remove(inLocation);
			SpawnedChunksMap.Remove(inLocation);
			FChunkRegion* region = RegionManager->GetChunkRegionByID(UChunkUtilityLib::GetRegionID(inLocation));
			region->ChunkRefCount--;
		}
	}
}
AChunk* AChunkManager::GetChunkAtWorldLocation(FVector location)
{
	TObjectPtr<AChunk>* chunk = SpawnedChunksMap.Find(UChunkUtilityLib::WorldLocationToChunkLocation(location));
	if(chunk)
	{
		return *chunk;
	}
	else
	{
		return nullptr;
	}
}
AChunk* AChunkManager::GetChunkAtLocation(FVector location)
{
	if(SpawnedChunksMap.Contains(location))
	{
		return SpawnedChunksMap[location];
	}
	return nullptr;
	
}

void AChunkManager::SetTickEnabled(bool bEnabled)
{
	SetActorTickEnabled(bEnabled);
}
void AChunkManager::EndPlay(EEndPlayReason::Type reason)
{
	ChunkMeshingTP->Destroy();
	delete ChunkMeshingTP;
	RegionManager->EndPlay();
	UChunkUtilityLib::ChunkManager = nullptr;
}

