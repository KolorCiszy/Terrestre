// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Chunk.h"
#include "ChunkUtilityLib.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Terrestre/Core/Block/BlockData.h"
#include "Terrestre/Core/Player/PlayerCharacter.h"
#include "Terrestre/Core/Subsystems/GameSavingSubsystem.h"
#include "Terrestre/Core/Gamemode/TerrestrePlayerState.h"
#include "Terrestre/Core/Player/PlayerData.h"
#include "Terrestre/Core/Gamemode/TerrestreGameModeBase.h"
#include "Terrestre/Core/Data Generators/TerrainShaper.h"
#include "Terrestre/Core/Data Generators/TerrainSurfaceDecorator.h"
#include "Terrestre/Core/Chunk/Async/GenerateChunkRegionDataTask.h"



AChunkManager::AChunkManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;
	SetActorTickEnabled(false);
	bReplicates = true;
	SetRole(ENetRole::ROLE_Authority);
	
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

	double terrainHeight = TerrainShaper->GetTerrainHeightAtXY_Native(0, 0);

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
	/*
	if (UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(this))
	{
		GSS = gameInstance->GetSubsystem<UGameSavingSubsystem>();	// Game Saving Subsystem

		playerState = Cast<ATerrestrePlayerState>(UGameplayStatics::GetPlayerState(this, 0));
		bShoudlLoadPlayerData = GSS->DoesPlayerDataExist(playerState->GetPlayerName());
	}
	if (!bShoudlLoadPlayerData)
	{
		currentChunkLocation = FVector::ZeroVector;
	}
	else
	{
		TSharedPtr<FPlayerData> playerData(GSS->LoadPlayerData(playerState->GetPlayerName()));
		if (playerData)
		{
			currentChunkLocation = UChunkUtilityLib::WorldLocationToChunkLocation(playerData->Transform.GetLocation());
		}
	}
	*/
}
void AChunkManager::CreateChunkRegionGenerationThreadPool()
{
	int32 threadsToCreate = FPlatformMisc::NumberOfCoresIncludingHyperthreads() - 2;
	if (threadsToCreate <= 0)
	{
		return;
	}
	ChunkRegionGenerationTP = FQueuedThreadPool::Allocate();
	if (ChunkRegionGenerationTP)
	{
		ChunkRegionGenerationTP->Create(threadsToCreate, 128 * 1024, EThreadPriority::TPri_Normal, TEXT("Chunk Region Data Generation Thread Pool"));
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
	BlockData::Initialize(); 
	UChunkUtilityLib::ChunkManager = this;
	TerrainShaper = NewObject<UTerrainShaper>();
	TerrainShaper->Initialize();
	TerrainSurfaceDecorator = NewObject<UTerrainSurfaceDecorator>();
	CreateChunkRegionGenerationThreadPool();
	CreateChunkMeshingThreadPool();
	GenerateStartingLocation();
	GenerateSpawnLocations(currentChunkLocation);
	RecalculateActiveRegions();
	for (auto& regionID : RegionsToLoad)
	{
		AsyncGenerateRegionData(regionID);
	}
	RegionsToLoad.Empty();
	while(!RegionsCurrentlyGenerating.IsEmpty())
	{
		FIntVector regionID;
		if (RegionsToGetDataFrom.Dequeue(regionID))
		{
			GetRegionDataAndDeleteTask(regionID);
		}
	}
	SpawnChunkActorPool();
	bShouldRecalculateActiveChunks = false;
	OnRebuildChunkMeshes.Broadcast();
	bool bMeshesReady{};
	while(ActiveMeshingTasksCount > 0)
	{
		FPlatformProcess::Sleep(0.5f);
	}
	TObjectPtr<AChunk> ApplyMeshChunk;
	while(ApplyChunkMeshQueue.Peek(ApplyMeshChunk))
	{
		if (ApplyMeshChunk->ApplyMesh())
		{
			ApplyChunkMeshQueue.Dequeue(ApplyMeshChunk);
		}
	}

	
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
	FChunkRegion* region = ActiveRegionsMap.Find(UChunkUtilityLib::GetRegionID(chunkLocation));
	if(region)
	{
		FIntVector localBlockPos = UChunkUtilityLib::WorldLocationToLocalBlockPos(worldLocation);
		return region->Data[chunkLocation].GetBlockAtIndex(UChunkUtilityLib::LocalBlockPosToIndex(localBlockPos));
	}
	return FBlockState();

}

FBlockPalette* AChunkManager::GetChunkBlockPalette(FVector chunkLocation)
{
	FScopeLock lock(&mutex);
	FChunkRegion* region = ActiveRegionsMap.Find(UChunkUtilityLib::GetRegionID(chunkLocation));
	if(region)
	{
		return &region->Data[chunkLocation];
	}
	return nullptr;	
}
bool AChunkManager::BulkUnpackChunkBlocks(FVector chunkLocation, TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& output)
{
	FScopeLock lock(&mutex);
	FChunkRegion* region = ActiveRegionsMap.Find(UChunkUtilityLib::GetRegionID(chunkLocation));
	if (region)
	{
		region->Data[chunkLocation].BulkUnpack(output);
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
	//* sort the locations, so that chunks closer to the player are first in the set
	/*	SpawnedChunksLocations.Sort([playerCurrentChunk](FVector a, FVector b)
		{
			return (FVector::DistSquared(playerCurrentChunk, a) > FVector::DistSquared(playerCurrentChunk, b));
		});
	Algo::Reverse(SpawnedChunksLocations);
	
	*/
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
			//mapEntry.Value->MarkPendingDestroy();
			LocationsToDespawn.Add(mapEntry.Key);
		}
	}
	bShouldRecalculateActiveChunks = false;
}

void AChunkManager::RecalculateActiveRegions() 
{
	ActiveRegionsIDs.Reset();
	FIntVector currentRegionID = UChunkUtilityLib::GetRegionID(currentChunkLocation);
	for(int8 x = -RegionLoadDistance; x <= RegionLoadDistance; x++)
	{
		for (int8 y = -RegionLoadDistance; y <= RegionLoadDistance; y++)
		{
			for (int8 z = -RegionLoadDistance; z <= RegionLoadDistance; z++)
			{
				FIntVector regionID = currentRegionID + FIntVector{ x,y,z };
				ActiveRegionsIDs.Add(regionID);
			}
		}
	}
	for(auto& regionID : ActiveRegionsIDs)
	{
		if (!ActiveRegionsMap.Contains(regionID)) 
		{
			RegionsToLoad.Add(regionID);
		}
	}
	for(auto& region : ActiveRegionsMap)
	{
		if (!ActiveRegionsIDs.Contains(region.Key))
		{
			RegionsToUnload.Add(region.Key);
		}
	}
};

void AChunkManager::AsyncGenerateRegionData(FIntVector regionID)
{
	RegionGenerationTasks.Add(regionID, new FAsyncTask<FGenerateChunkRegionDataTask>(regionID));
	RegionGenerationTasks[regionID]->StartBackgroundTask(ChunkRegionGenerationTP);
	RegionsCurrentlyGenerating.Add(regionID);
}
void AChunkManager::RegionDataGenerationComplete(FIntVector regionID)
{
	RegionsToGetDataFrom.Enqueue(regionID);
}

void AChunkManager::GetRegionDataAndDeleteTask(FIntVector regionID)
{
	if (RegionGenerationTasks[regionID]->IsDone())
	{
		FScopeLock lock(&mutex);
		ActiveRegionsMap.Add(regionID);
		auto& task = RegionGenerationTasks[regionID]->GetTask();
		ActiveRegionsMap[regionID] = MoveTemp(task.newRegion);
		ActiveRegionsMap[regionID].RefCount = 0;
		RegionsCurrentlyGenerating.Remove(regionID);
		delete RegionGenerationTasks[regionID];
		RegionGenerationTasks.Remove(regionID);
	};
}

void AChunkManager::SetRenderDistance(uint8 newDistance)
{
	RenderDistance = FMath::Clamp(newDistance, MinRenderDistance, MaxRenderDistance);
	GenerateSpawnLocations(currentChunkLocation);
}
bool AChunkManager::CheckIfTickTooLong()
{
	tickEnd = std::chrono::high_resolution_clock::now();
	tickTime = tickEnd - tickStart;
	if (tickTime >= std::chrono::milliseconds(13))
	{
#ifdef UE_BUILD_DEBUG
		check(tickTime <= std::chrono::milliseconds(600))
#endif
		return true;
	}
	return false;
}
void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/* Start measuring tick time to know when to skip a tick if it takes too long to process */
	tickStart = std::chrono::high_resolution_clock::now();


	TObjectPtr<AChunk> ApplyMeshChunk{};
	while (ApplyChunkMeshQueue.Peek(ApplyMeshChunk))
	{
		if (ApplyMeshChunk->ApplyMesh())
		{
			ApplyChunkMeshQueue.Dequeue(ApplyMeshChunk);
		}

	}

	OnRebuildChunkMeshes.Broadcast();

	

	if (bShouldRecalculateActiveChunks)
	{
		RecalculateActiveRegions();
		RecalculateActiveChunks();
	}
	
	if (auto regionToGenerate = RegionsToLoad.CreateIterator())
	{
		if (!RegionsCurrentlyGenerating.Contains(*regionToGenerate))
		{
			AsyncGenerateRegionData(*regionToGenerate);
			RegionsToLoad.Remove(*regionToGenerate);
		}
	}	
	

	TSet<FVector> removedChunkDespawnLocations;
	FIntVector getThisRegion;
	if (RegionsToGetDataFrom.Dequeue(getThisRegion))
	{
		GetRegionDataAndDeleteTask(getThisRegion);
	}
	
	for (auto& oldLocation : LocationsToDespawn)
	{
		
		
		const FVector& newLocation = *LocationsToSpawn.CreateConstIterator();
		FIntVector newRegionID = UChunkUtilityLib::GetRegionID(newLocation);
		FIntVector oldRegionID = UChunkUtilityLib::GetRegionID(oldLocation);
		FChunkRegion* newRegion = ActiveRegionsMap.Find(newRegionID);
		FChunkRegion* oldRegion = ActiveRegionsMap.Find(oldRegionID);
		
  		
		TObjectPtr<AChunk> chunk;
		chunk = SpawnedChunksMap[oldLocation];
		if (newRegion)
		{
			chunk->ClearMeshSection(0);
			chunk->SetActorLocation(newLocation);
			chunk->MarkMeshDirty();
			SpawnedChunksMap.Add(newLocation, chunk);
			SpawnedChunksMap.Remove(oldLocation);

			if (newRegionID == oldRegionID)
			{
				ActiveRegionsMap[newRegionID].RefCount++;
			}
			else
			{
				ActiveRegionsMap[newRegionID].RefCount++;
				check(ActiveRegionsMap[oldRegionID].RefCount >= 0)
				ActiveRegionsMap[oldRegionID].RefCount--;
			}
			LocationsToSpawn.Remove(newLocation);
			removedChunkDespawnLocations.Add(oldLocation);
		}
		
	}
	for (auto& location : removedChunkDespawnLocations)
	{
		LocationsToDespawn.Remove(location);
	
	}
	
	TSet<FIntVector> removedRegions;
	for (auto& regionID : RegionsToUnload)
	{
		if (FChunkRegion* region = ActiveRegionsMap.Find(regionID))
		{
			if(region->RefCount == 0)
			{
				FScopeLock lock(&mutex);
				ActiveRegionsMap.Remove(regionID);
				removedRegions.Add(regionID);
			}
		}
	}
	RegionsToUnload = RegionsToUnload.Difference(removedRegions);
	
}
TObjectPtr<AChunk> AChunkManager::SpawnChunkAtLocation(const FVector inLocation)
{
	FTransform spawnTransform{inLocation};
	TObjectPtr<AChunk> chunk(GetWorld()->SpawnActor<AChunk>(ChunkClass, spawnTransform));
	FChunkRegion& region = ActiveRegionsMap[(UChunkUtilityLib::GetRegionID(inLocation))];
	SpawnedChunksMap.Add(inLocation, chunk);
	region.RefCount++;
	return chunk;
}
void AChunkManager::DestroyChunkAtLocation(const FVector inLocation)
{
	if(TObjectPtr<AChunk>* chunk = SpawnedChunksMap.Find(inLocation))
	{
		if(chunk->Get()->bReadyToDestroy.load())
		{
			chunk->Get()->Destroy();
			LocationsToDespawn.Remove(inLocation);
			SpawnedChunksMap.Remove(inLocation);
			ActiveRegionsMap[UChunkUtilityLib::GetRegionID(inLocation)].RefCount--;
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
	ChunkRegionGenerationTP->Destroy();
	delete ChunkRegionGenerationTP;
	for(auto& task : RegionGenerationTasks)
	{
		if (task.Value->IsDone()) 
		{
			delete task.Value;
		}
		else
		{
			task.Value->Cancel();
			task.Value->EnsureCompletion();
			delete task.Value;
		}
	}
	RegionGenerationTasks.Empty();
}

