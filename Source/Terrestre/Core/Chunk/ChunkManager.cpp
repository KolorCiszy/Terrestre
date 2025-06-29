// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Chunk.h"
#include "ChunkConstants.h"
#include "ChunkUtilityLib.h"
#include "Kismet/GameplayStatics.h"
#include "Terrestre/Core/Character/Player/PlayerCharacter.h"
#include "Terrestre/Core/Gamemode/TerrestrePlayerState.h"
#include "Terrestre/Core/Gamemode/TerrestreGameModeBase.h"
#include "Terrestre/Core/Gamemode/TerrestreGameInstance.h"
#include <Terrestre/Core/Subsystems/WorldGenSubsystem/WorldGenSubsystem.h>



AChunkManager::AChunkManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;
	
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

TFuture<FChunkData> AChunkManager::ProvideChunkData(AChunk* Chunk)
{
	// check if can load from disk using saving subsystem, if not:
	if (Chunk->IsValidLowLevel())
	{
		return GetGameInstance<UTerrestreGameInstance>()->GetSubsystem<UWorldGenSubsystem>()->ProvideChunkData(Chunk);
	}
	return TFuture<FChunkData>();
}

FVector AChunkManager::AdjustPlayerSpawnLocation(FVector initialLocation)
{
	if (auto Chunk = GetChunkAtWorldLocation(initialLocation))
	{
		FVector SpawnLoc{};
		uint8 AirBlocksInRow{};
		for (int z{}; z <FChunkConstants::Size; z++)
		{
			FIntVector currentLocalPos{ 0,0, z };
			auto Block = Chunk->GetBlockAtLocalPosition(currentLocalPos);
			if (Block.IsAirBlock())
			{
				AirBlocksInRow++;
			}
			else
			{
				AirBlocksInRow = 0;
			}
			if (AirBlocksInRow == 3)
			{
				FIntVector SpawnLocalPos{ currentLocalPos.X, currentLocalPos.Y, currentLocalPos.Z };
				SpawnLoc = UChunkUtilityLib::LocalPositionToWorldLocation(SpawnLocalPos, Chunk->GetActorLocation());
				initialLocation.Z = SpawnLoc.Z;
				return initialLocation;
			}
		}
			
	}
	return (initialLocation);
}



void AChunkManager::BeginPlay()
{
	Super::BeginPlay();
	ChunkMeshingTP = GetGameInstance<UTerrestreGameInstance>()->GetChunkMeshingTP();
	
	SetupSpawnChunks();

	// wait until all data is gathered
	bool bReady = false;

	
	GetWorld()->GetTimerManager().SetTimer(
		SpawnChunksTimerHandle,
		FTimerDelegate::CreateLambda([this]()
			{
				
				if (ActiveMeshingTasksCount == 0 && ActiveChunkDataProviderTasksCount == 0)
				{
					OnSpawnChunksReady.Broadcast();

					SetActorTickEnabled(true);
					auto GM = Cast<ITerrestreGameModeInterface>(GetWorld()->GetAuthGameMode());
					if (!GM)
					{
						UE_LOG(LogTemp, Error, TEXT("Chunk manager could not get game mode interface"));
						return;
					}
					
					GM->RequestPlayerPawnSpawn(GetWorld()->GetFirstPlayerController());
					GetWorld()->GetTimerManager().ClearTimer(SpawnChunksTimerHandle);
				}
			}),
		0.2f, // Rate in seconds
		true,
		0.2f// Looping
	);
	
	
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
	bShouldRecalculateActiveChunks = true;
}





void AChunkManager::SetupSpawnChunks()
{
	auto GameMode = GetWorld()->GetAuthGameMode<ATerrestreGameModeBase>();
	
	int32 radius = GameMode->SpawnChunkRadius;
	
	for (int32 x = -radius; x <= radius; x++)
	{
		for (int32 y = -radius; y <= radius; y++)
		{
			for (int32 z = -radius; z <= radius; z++)
			{
				FVector chunkWorldLocation{ x * FChunkConstants::SizeScaled.X + GameMode->DefaultSpawnChunkStartLocation.X,
											y * FChunkConstants::SizeScaled.Y + GameMode->DefaultSpawnChunkStartLocation.Y,
											z * FChunkConstants::SizeScaled.Z + GameMode->DefaultSpawnChunkStartLocation.Z };

				SpawnChunkAtLocation(chunkWorldLocation, false);
			}
		}
	}

}

void AChunkManager::RecalculateActiveChunks()
{
	auto GameMode = GetWorld()->GetAuthGameMode<ATerrestreGameModeBase>();
	// Determine target chunks to be loaded
	for (int32 X = -GameMode->RenderDistance; X <= GameMode->RenderDistance; ++X)
	{
		for (int32 Y = -GameMode->RenderDistance; Y <= GameMode->RenderDistance; ++Y)
		{
			for (int32 Z = -GameMode->RenderDistance; Z <= GameMode->RenderDistance; ++Z)
			{
				bool bBorderChunk{};
				FVector TargetLoc = currentChunkLocation + FVector(X, Y, Z) * FChunkConstants::SizeScaled;
				if (FMath::Abs(X) == GameMode->RenderDistance || FMath::Abs(Y) == GameMode->RenderDistance || FMath::Abs(Z) == GameMode->RenderDistance)
				{
					bBorderChunk = true;
				}
				SpawnedChunksLocations.Add(TargetLoc, bBorderChunk);
			}
		}
	}

	LocationsToSpawn.Empty(FMath::Pow(GameMode->RenderDistance, 3.0));
	LocationsToDespawn.Empty(FMath::Pow(GameMode->RenderDistance, 3.0));

	for (auto& location : SpawnedChunksLocations)
	{
		if (!SpawnedChunksMap.Contains(location.Key))
		{
			LocationsToSpawn.Add(location.Key);  // TODO promote chunks to not border chunks, check locations
		}
		else
		{
			//SpawnedChunksMap[location.Key]->bBorderChunk = location.Value;
			SpawnedChunksMap[location.Key]->ChangeBorderChunkStatus(location.Value);// Update border chunk status TODO: Make this into a function on AChunk so that it knows to update its gen state and mark mesh dirty
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




void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldRecalculateActiveChunks)
	{
		RecalculateActiveChunks();
	}
	TSet<FVector> ChunksLocationsLocal{};
	uint8 SpawnCounter{};
	for (auto& Loc : LocationsToSpawn)
	{
		SpawnChunkAtLocation(Loc, SpawnedChunksLocations[Loc]);
		SpawnCounter++;
		ChunksLocationsLocal.Add(Loc);
		if (SpawnCounter >= GetWorld()->GetAuthGameMode<ATerrestreGameModeBase>()->ChunksToSpawnPerTick)
		{
			break;
		}
	}
	LocationsToSpawn = LocationsToSpawn.Difference(ChunksLocationsLocal);

	ChunksLocationsLocal.Empty();
	SpawnCounter = 0;
	
	
	for (auto& Loc : LocationsToDespawn)
	{
		DestroyChunkAtLocation(Loc);
		ChunksLocationsLocal.Add(Loc);
		SpawnCounter++;
		if (SpawnCounter >= GetWorld()->GetAuthGameMode<ATerrestreGameModeBase>()->ChunksToDespawnPerTick)
		{
			break;
		}
	}
	LocationsToDespawn = LocationsToDespawn.Difference(ChunksLocationsLocal);
	
		/*
	TSet<FVector> removedChunkDespawnLocations{};
	int32 movedChunks = 0;
	for (auto& oldLocation : LocationsToDespawn)
	{
		const FVector& newLocation = *LocationsToSpawn.CreateConstIterator();
		FIntVector newRegionID = UChunkUtilityLib::GetRegionID(newLocation);
		FIntVector oldRegionID = UChunkUtilityLib::GetRegionID(oldLocation);
		//FChunkRegion* newRegion = RegionManager->GetChunkRegionByID(newRegionID);
		//FChunkRegion* oldRegion = RegionManager->GetChunkRegionByID(oldRegionID);
		
  		
		TObjectPtr<AChunk> chunk;
		chunk = SpawnedChunksMap[oldLocation];
		if (chunk->bMeshingTaskDone)
		{
			chunk->ResetMesh();
			chunk->SetActorLocation(newLocation);
			//chunk->MarkMeshDirty();
			SpawnedChunksMap.Add(newLocation, chunk);
			SpawnedChunksMap.Remove(oldLocation);
			/*
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
			*/
			//LocationsToSpawn.Remove(newLocation);
			//removedChunkDespawnLocations.Add(oldLocation);
		//}
		
		//movedChunks++;
		//if (movedChunks >= GetWorld()->GetAuthGameMode<ATerrestreGameModeBase>()->ChunksToMovePerTick)
		
	//}
	
	/*
	for (auto& location : removedChunkDespawnLocations)
	{
		LocationsToDespawn.Remove(location);
	}
	*/
	
	
}
AChunk* AChunkManager::SpawnChunkAtLocation(const FVector inLocation, bool bBorderChunk)
{
	FTransform spawnTransform{inLocation};
	AChunk* Chunk(GetWorld()->SpawnActorDeferred<AChunk>(ChunkClass, spawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	
	Chunk->bBorderChunk = bBorderChunk;
	SpawnedChunksMap.Add(inLocation, Chunk);
	UGameplayStatics::FinishSpawningActor(Chunk, spawnTransform);
	
	return Chunk;
}
void AChunkManager::DestroyChunkAtLocation(const FVector inLocation)
{
	if(TObjectPtr<AChunk>* chunk = SpawnedChunksMap.Find(inLocation))
	{
		if(chunk->Get()->bReadyToDestroy)
		{
			chunk->Get()->Destroy();
			
			SpawnedChunksMap.Remove(inLocation);
		}
		else
		{
			chunk->Get()->MarkPendingDestroy();
			UE_LOG(LogTemp, Warning, TEXT("Chunk at %s marked for destroy but wasn't ready for it"), *inLocation.ToString());
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
	Super::EndPlay(reason);
	UE_LOG(LogTemp, Warning, TEXT("Chunk manager EndPlay"));

	for(auto& chunk : SpawnedChunksMap)
	{
		if (chunk.Value->IsValidLowLevel())
		{
			DestroyChunkAtLocation(chunk.Key);
		}
	}


}

