// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <chrono>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Storage/ChunkRegion.h"
#include "Terrestre/Core/Data Generators/FastNoiseSettings.h"
#include "Terrestre/Core/Player/PlayerCharacter.h"
#include "ChunkManager.generated.h"

class AChunk;
class UDataRegistry;
class ATerrestrePlayerState;
class UTerrainShaper;
class UTerrainSurfaceDecorator;
class FGenerateChunkRegionDataTask;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateChunksMeshDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FApplyChunksMeshDelegate);

UCLASS()
class TERRESTRE_API AChunkManager : public AActor
{
	GENERATED_BODY()
	
public:	
	
	friend void APlayerCharacter::RegisterCharacterToWorld();
	friend void APlayerCharacter::UnRegisterCharacterToWorld();
	friend FBlockPalette* AChunk::GetBlockPalette() const;
	friend void AChunk::CreateMeshAsync();
	friend class FGenerateChunkRegionDataTask;
	friend class UTerrainShaper;

	static constexpr uint8 MaxRenderDistance = 16;
	static constexpr uint8 MinRenderDistance = 0;

	AChunkManager();
	/* How many chunks will, be fully visible from the chunk player is currently in*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chunk rendering")
	uint8 RenderDistance = 3;

	/* How many regions will the game pre load from the region the player is currently in */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk region data")
	uint8 RegionLoadDistance = 1;

	/* How many chunks can spawn during one chunk manager tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk manager")
	uint8 ChunksToSpawnPerTick = 10;

	/* How many chunks can despawn during one chunk manager tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk manager")
	uint8 ChunksToDespawnPerTick = 15;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk manager")
	FVector DefaultPlayerSpawnLocation;

	UFUNCTION(BlueprintCallable, Category = "Chunk manager")
	void SetRenderDistance(uint8 newDistance);

	UFUNCTION(BlueprintNativeEvent, Category = "Chunk manager")
	void OnPlayerLocationChanged(FVector currentLocation);
	void OnPlayerLocationChanged_Implementation(FVector currentLocation);

	/* In world location gets snapped to chunk grid*/
	UFUNCTION(BlueprintCallable, Category = "Chunk manager")
	AChunk* GetChunkAtWorldLocation(FVector location);

	UFUNCTION(BlueprintCallable, Category = "Chunk manager")
	FBlockState GetBlockAtWorldLocation(FVector worldLocation);

	/* Passed location has to be the exact location of a chunk, which is faster than finding by world location */
	AChunk* GetChunkAtLocation(FVector location);

	FBlockPalette* GetChunkBlockPalette(FVector chunkLocation);
	/* Thread safe */
	bool BulkUnpackChunkBlocks(FVector chunkLocation, TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& output);

	UPROPERTY(BlueprintAssignable)
	FUpdateChunksMeshDelegate OnRebuildChunkMeshes;

	UPROPERTY(BlueprintAssignable)
	FApplyChunksMeshDelegate OnApplyChunkMeshes;

	UPROPERTY(EditDefaultsOnly)
	FFastNoiseSettings terrainShaperDensityNoise;

	UPROPERTY(EditDefaultsOnly)
	FFastNoiseSettings terrainShaperContinentalnessNoise;

	std::atomic<int32> ActiveMeshingTasksCount;

	TQueue<TObjectPtr<AChunk>> ApplyChunkMeshQueue;

	void SetTickEnabled(bool bEnabled);
	/* Locations which should contain spawned chunks according to player's location */
	TArray<FVector> SpawnedChunksLocations;
	/* Map for currently spawned chunks */
	TMap<FVector, TObjectPtr<AChunk>> SpawnedChunksMap;
	/* Holds region data */
	TMap<FIntVector, FChunkRegion> ActiveRegionsMap;

	TSet<FIntVector> ActiveRegionsIDs;

	TSet<FIntVector> RegionsToLoad;

	TSet<FIntVector> RegionsToUnload;

	TSet<FVector> LocationsToSpawn;

	TSet<FVector> LocationsToDespawn;

	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type reason) override;
private:	
	
	void CreateChunkMeshingThreadPool();

	/* Set currentChunkLocaiton to either zero or the locaiton player was last in */
	void GenerateStartingLocation();
	/* Registers a player character so the chunk manager can bind to OnPlayerLocationChanged delegate */
	void RegisterPlayerCharacter(TObjectPtr<APlayerCharacter> player);
	/* Unregisters a player character so the chunk manager doesn't care about its location */
	void UnRegisterPlayerCharacter(TObjectPtr<APlayerCharacter> player);
	
	virtual void Tick(float DeltaTime) override;

	/* Cheks it tick time is too long, if so the tick function can return early 
	 * ONLY CALL INSIDE TICK FUNCTION
	 */
	inline bool CheckIfTickTooLong();
	/* playerCurrentChunk - the player is located in this chunk */
	void GenerateSpawnLocations(FVector playerCurrentChunk);
	/* Only called after the world is loaded (only called by tick function) */
	void RecalculateActiveChunks();
	/* Called alongside with RecalculateActiveChunks, load or unloades regions during tick or beginplay */
	void RecalculateActiveRegions();

	void CreateChunkRegionGenerationThreadPool();
	/* Called to async generate region data for a given regionID */
	void AsyncGenerateRegionData(FIntVector regionID);
	/* Called from worker thread to inform the manager that the works is finished*/
	void RegionDataGenerationComplete(FIntVector regionID);

	void GetRegionDataAndDeleteTask(FIntVector regionID);

	void SpawnChunkActorPool();


	/* The current Chunk that player is in */
	FVector currentChunkLocation;
	
	TSet<FIntVector> RegionsCurrentlyGenerating;

	TQueue<FIntVector> RegionsToGetDataFrom;

	TMap<FIntVector, FAsyncTask<FGenerateChunkRegionDataTask>*> RegionGenerationTasks;

	UPROPERTY()
	TObjectPtr<UTerrainShaper> TerrainShaper;

	UPROPERTY()
	TObjectPtr<UTerrainSurfaceDecorator> TerrainSurfaceDecorator;

	TObjectPtr<AChunk> SpawnChunkAtLocation(const FVector inLocation);

	void DestroyChunkAtLocation(const FVector inLocation);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AChunk> ChunkClass;
	
	bool bShouldRecalculateActiveChunks;

	FCriticalSection mutex;

	FQueuedThreadPool* ChunkMeshingTP;

	FQueuedThreadPool* ChunkRegionGenerationTP;

	std::chrono::steady_clock::time_point tickStart;;
	decltype(tickStart) tickEnd;
	std::chrono::duration<double, std::milli> tickTime; //= std::chrono::duration<double, std::milli>(end - start);
};
