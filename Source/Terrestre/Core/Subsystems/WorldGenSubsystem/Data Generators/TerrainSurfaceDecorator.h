#pragma once

#include "CoreMinimal.h"
#include "RegionGenerationData.h"
#include "TerrainSurfaceDecorator.generated.h"

struct FChunkProtoRegion;
struct FBlockState;
class UTerrainSurfaceDecoratorSettings;
class AChunk;
class FChunkData;

UCLASS()
class TERRESTRE_API UTerrainSurfaceDecorator : public UObject
{
	GENERATED_BODY()

public:
	

	void Initialize();

	UPROPERTY()
	TObjectPtr<UTerrainSurfaceDecoratorSettings> Settings;

	bool GenerateSurfaceLayer(AChunk* Chunk, FChunkData& DataCopy);

	bool GenerateTerrainSurfaceDecorations(FChunkProtoRegion& protoRegion, FIntVector RegionID);
};