#pragma once

#include "CoreMinimal.h"
#include "TerrainSurfaceDecorator.generated.h"

struct FBlockState;

UCLASS()
class UTerrainSurfaceDecorator : public UObject
{
	GENERATED_BODY()

public:
	static void GenerateTerrainSurfaceDecorations(TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& chunkBlocks, FVector chunkLocation, bool bDensityHeightChange, TArray<int16>& heightMap);
};