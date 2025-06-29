// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Terrestre/Core/Subsystems/GameSavingSubsystem/WorldSaveData.h"
#include "TerrestreGameInstance.generated.h"



class UWorldSaveData;
class UTerrainShaperSettings;
class UTerrainSurfaceDecoratorSettings;
/**
 * 
 */
UCLASS(Config=Game)
class TERRESTRE_API UTerrestreGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;

	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, Exec, Category = "Chunk manager commands")
	void SetChunkManagerTickEnabled(bool bEnabled);
	
	UFUNCTION(BlueprintCallable, Exec, Category = "Chunk manager commands")
	void SetRenderDistance(uint8 newDistance);
	
	/* Loads world from given world save data */
	UFUNCTION(BlueprintCallable, Category = "Terrestre Game Instance")
	void LoadWorld(UWorldSaveData* WorldToLoad);
	/* Unloads currently open world, don't call when there is no world loaded */
	UFUNCTION(BlueprintCallable, Category = "Terrestre Game Instance")
	void UnloadWorld();


	void ReceivePreLoadMap(const FString& InMapName);

	void ReceivePostLoadMapWithWorld(UWorld* InLoadedWorld);

	const FName& GetPrimaryDimensionName() const
	{
		return PrimaryDimensionName;
	};
	const UWorldSaveData* const GetOpenWorldData() const
	{
		return OpenWorldData;
	}
	int32 GetWorldGenThreadCount() const
	{
		return WorldGenThreadCount;
	}
	FQueuedThreadPool* GetChunkMeshingTP() const
	{
		return ChunkMeshingTP;
	}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTerrainShaperSettings> TerrainShaperSettings;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTerrainSurfaceDecoratorSettings> TerrainSurfaceDecoratorSettings;
protected:
	/* The name of map (level) that should be treated as overworld (default dimension) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName PrimaryDimensionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config)
	int32 ChunkMeshingThreadCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config)
	int32 WorldGenThreadCount;

	/* Current open world save data (save name, world seed etc.) */
	UPROPERTY()
	TObjectPtr<UWorldSaveData> OpenWorldData;
	


	FQueuedThreadPool* ChunkMeshingTP;

	void SetupWorkerThreadPools();
};
