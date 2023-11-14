#pragma once

#include "CoreMinimal.h"
#include "RegionGenerationData.h"
#include "TerrainSurfaceDecorator.generated.h"

struct FChunkProtoRegion;
struct FBlockState;

UCLASS()
class UTerrainSurfaceDecorator : public UObject
{
	GENERATED_BODY()

public:
	

	bool GenerateTerrainSurfaceDecorations(FChunkProtoRegion& protoRegion, FIntVector RegionID);
};