// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <chrono>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Storage/ChunkRegion.h"
#include "Terrestre/Core/Data Generators/FastNoiseSettings.h"
#include "Terrestre/Core/Character/Player/PlayerCharacter.h"
#include "Storage/RegionManager.h"
#include "ChunkManager.generated.h"


class AChunk;
class ATerrestrePlayerState;
class UDataRegistry;
class UTerrainShaperSettings;
class UTerrainShaper;
class UTerrainSurfaceDecorator;



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateChunksMeshDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FApplyChunksMeshDelegate);

UCLASS(Config=Game)
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
	friend URegionManager* URegionManager::Get();

	static constexpr uint8 MaxRenderDistance = 16;
	static constexpr uint8 MinRenderDistance = 0;

	AChunkManager();

	/* How many chunks will be fully visible from the chunk player is currently in*/
	UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Chunk rendering")
	uint8 RenderDistance;

	/* How many regions will the game pre load from the region the player is currently in, all of them will have at least the first step of generation complete */
	UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Chunk region data")
	uint8 RegionLoadDistance;


	/* How many chunks can be moved during one chunk manager tick */
	UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Chunk manager")
	uint8 ChunksToMovePerTick;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk manager")
	FVector DefaultPlayerSpawnLocation;

	UPROPERTY(BlueprintAssignable)
	FUpdateChunksMeshDelegate OnRebuildChunkMeshes;

	UPROPERTY(BlueprintAssignable)
	FApplyChunksMeshDelegate OnApplyChunkMeshes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTerrainShaperSettings> TerrainShaperSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AChunk> ChunkClass;

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

	UFUNCTION(BlueprintCallable, Category = "Chunk manager")
	FFluidState GetFluidAtWorldLocation(FVector worldLocation);

	/* Passed location has to be the exact location of a chunk, which is faster than finding by world location */
	AChunk* GetChunkAtLocation(FVector location);

	FBlockPalette* GetChunkBlockPalette(FVector chunkLocation);

	bool GetChunkFluidStates(FVector chunkLocation, TArray<FFluidState, TInlineAllocator<AChunk::Volume>>& fluidStates);

	/* Thread safe */
	bool BulkUnpackChunkBlocks(FVector chunkLocation, TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& output);

	void SetTickEnabled(bool bEnabled);

	/******** CHUNK ********/

	std::atomic<int32> ActiveMeshingTasksCount;

	TQueue<TObjectPtr<AChunk>, EQueueMode::Mpsc> ApplyChunkMeshQueue;

	/* Locations which should contain spawned chunks according to player's location */
	TArray<FVector> SpawnedChunksLocations;

	/* Map for currently spawned chunks */
	TMap<FVector, TObjectPtr<AChunk>> SpawnedChunksMap;

	TSet<FVector> LocationsToSpawn;

	TSet<FVector> LocationsToDespawn;

	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type reason) override;


	UPROPERTY()
	TObjectPtr<UTerrainShaper> TerrainShaper;

	UPROPERTY()
	TObjectPtr<UTerrainSurfaceDecorator> TerrainSurfaceDecorator;

	UPROPERTY()
	TObjectPtr<URegionManager> RegionManager;

private:	
	
	void CreateChunkMeshingThreadPool();

	/* Set currentChunkLocaiton to either zero or the locaiton player was last in */
	void GenerateStartingLocation();
	/* Registers a player character so the chunk manager can bind to OnPlayerLocationChanged delegate */
	void RegisterPlayerCharacter(TObjectPtr<APlayerCharacter> player);
	/* Unregisters a player character so the chunk manager doesn't care about its location */
	void UnRegisterPlayerCharacter(TObjectPtr<APlayerCharacter> player);
	
	virtual void Tick(float DeltaTime) override;

	/* playerCurrentChunk - the player is located in this chunk */
	void GenerateSpawnLocations(FVector playerCurrentChunk);
	/* Only called after the world is loaded (only called by tick function) */
	void RecalculateActiveChunks();
	/* Called alongside with RecalculateActiveChunks, load or unloades regions during tick or beginplay */
	//void RecalculateActiveRegions();

	void UpdateRegionsToLoad();

	void UpdateRegionsToUnload();
	
	void SpawnChunkActorPool();

	/* The current Chunk that player is in */
	FVector currentChunkLocation;


	TObjectPtr<AChunk> SpawnChunkAtLocation(const FVector inLocation);

	void DestroyChunkAtLocation(const FVector inLocation);

	
	
	bool bShouldRecalculateActiveChunks;

	FQueuedThreadPool* ChunkMeshingTP;

	

	std::chrono::steady_clock::time_point tickStart;
	decltype(tickStart) tickEnd;
	std::chrono::duration<double, std::milli> tickTime; //= std::chrono::duration<double, std::milli>(end - start);
};
