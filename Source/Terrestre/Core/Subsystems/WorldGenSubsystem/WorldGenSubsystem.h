#pragma once

#include "CoreMinimal.h"
#include "Terrestre/Core/Chunk/ChunkProviderInterface.h"
#include "WorldGenSubsystem.generated.h"

class UTerrainShaper;
class UTerrainSurfaceDecorator;
class FGenerateChunkRegionDataTask;

UCLASS()
class UWorldGenSubsystem : public UGameInstanceSubsystem, public IChunkProviderInterface
{
public:

	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;
	
	
	void ReceivePreLoadMap(const FString& InMapName);

	void ReceivePostLoadMapWithWorld(UWorld* InLoadedWorld) {};

	TFuture<FChunkData> ProvideChunkData(AChunk* Chunk) override;
	
	void GenerateSpawnRegions(FIntVector CenterRegionID);

	static  FQueuedThreadPool* GetRegionGenPool();

	UFUNCTION(BlueprintPure, Category="WorldGen")
	UTerrainShaper* GetTerrainShaper()
	{
		return TerrainShaper;
	};

protected:
	
	UPROPERTY()
	TObjectPtr<UTerrainShaper> TerrainShaper;
	UPROPERTY()
	TObjectPtr<UTerrainSurfaceDecorator> TerrainSurfaceDecorator;

	static inline FQueuedThreadPool* WorldGenTP;
	
	void CreateWorldGenThreadPool();
	
	TFuture<FChunkData> GenerateChunkTerrainShape(AChunk* Chunk);

	TFuture<FChunkData> GenerateChunkSurfaceLayer(AChunk* Chunk);
};