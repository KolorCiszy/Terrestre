// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <chrono>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Storage/ChunkRegion.h"
#include "Terrestre/Core/Character/Player/PlayerCharacter.h"
#include "ChunkProviderInterface.h"
#include "ChunkAccess.h"
#include "ChunkUtilityLib.h"
#include "ChunkManager.generated.h"

class AChunk;
class UDataRegistry;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpawnChunksReadyDelegate);

DECLARE_MULTICAST_DELEGATE(FApplyChunksMeshDelegate);

UCLASS()
class TERRESTRE_API AChunkManager : public AActor, public IChunkProviderInterface, public IChunkAccess
{
	GENERATED_BODY()
	
public:	

	AChunkManager();

	virtual TFuture<FChunkData> ProvideChunkData(AChunk* Chunk) override;
	
	UPROPERTY(BlueprintAssignable)
	FSpawnChunksReadyDelegate OnSpawnChunksReady;

	FBlockState GetBlockAtWorldPosition(const FVector& worldPosition) override
	{
		if (AChunk* chunk = GetChunkAtWorldLocation(worldPosition))
		{
			auto LocalPos = UChunkUtilityLib::WorldLocationToLocalBlockPos(worldPosition);
			return chunk->GetBlockAtLocalPosition(LocalPos);
		}
		return FBlockState::AirBlock();
	}
	FFluidState GetFluidAtWorldPosition(const FVector& worldPosition) override 
	{
		if (AChunk* chunk = GetChunkAtWorldLocation(worldPosition))
		{
			auto LocalPos = UChunkUtilityLib::WorldLocationToLocalBlockPos(worldPosition);
			return chunk->GetFluidAtLocalPosition(LocalPos);
		}
		return FFluidState::Empty();
	}

	FApplyChunksMeshDelegate OnApplyChunkMeshes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AChunk> ChunkClass;

	UFUNCTION(BlueprintCallable, Category = "Chunk manager")
		void SetRenderDistance(uint8 newDistance) {};

	UFUNCTION(BlueprintNativeEvent, Category = "Chunk manager")
	void OnPlayerLocationChanged(FVector currentLocation);
	void OnPlayerLocationChanged_Implementation(FVector currentLocation);

	/* In world location gets snapped to chunk grid*/
	UFUNCTION(BlueprintCallable, Category = "Chunk manager")
	AChunk* GetChunkAtWorldLocation(FVector location);


	/* Passed location has to be the exact location of a chunk, which is faster than finding by world location */
	AChunk* GetChunkAtLocation(FVector location);

	FBlockPalette* GetChunkBlockPalette(FVector chunkLocation);

	/* Adjusts player character spawn location according to current voxels that are present at give X and Y */
	FVector AdjustPlayerSpawnLocation(FVector initialLocation);

	void SetTickEnabled(bool bEnabled);

	/******** CHUNK ********/

	std::atomic<int32> ActiveMeshingTasksCount;

	std::atomic<int32> ActiveChunkDataProviderTasksCount;

	TQueue<TObjectPtr<AChunk>, EQueueMode::Mpsc> ApplyChunkMeshQueue;

	/* Locations which should contain spawned chunks according to player's location */
	/* Maps chunk location to bIsBorderChunk */
	TMap<FVector, bool> SpawnedChunksLocations;

	/* Map for currently spawned chunks */
	TMap<FVector, TObjectPtr<AChunk>> SpawnedChunksMap;

	TSet<FVector> LocationsToSpawn;

	TSet<FVector> LocationsToDespawn;

	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type reason) override;

	FQueuedThreadPool* ChunkMeshingTP;  

	/* Registers a player character so the chunk manager can bind to OnPlayerLocationChanged delegate */
	void RegisterPlayerCharacter(TObjectPtr<APlayerCharacter> player);
	/* Unregisters a player character so the chunk manager doesn't care about its location */
	void UnRegisterPlayerCharacter(TObjectPtr<APlayerCharacter> player);

private:	
	
	/* Set currentChunkLocaiton to either zero or the locaiton player was last in */
	void GenerateStartingLocation();
	
	virtual void Tick(float DeltaTime) override;

	/* Only called after the world is loaded (only called by tick function) */
	void RecalculateActiveChunks();
	/* Called alongside with RecalculateActiveChunks, load or unloades regions during tick or beginplay */
	//void RecalculateActiveRegions();
	
	/* The current Chunk that player is in */
	FVector currentChunkLocation;

	void SetupSpawnChunks();

	AChunk* SpawnChunkAtLocation(const FVector inLocation, bool bBorderChunk);

	void DestroyChunkAtLocation(const FVector inLocation);

	FTimerHandle SpawnChunksTimerHandle;
	
	bool bShouldRecalculateActiveChunks;
	
};
