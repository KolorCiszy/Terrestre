#pragma once

#include "CoreMinimal.h"

struct FRegionGenerationData
{
	FRegionGenerationData()
	{
		ContinentalnessValues.SetNumUninitialized(FChunkRegion::RegionSizeInBlocksSquared, false);
		ErosionValues.SetNumUninitialized(FChunkRegion::RegionSizeInBlocksSquared, false);
		PVValues.SetNumUninitialized(FChunkRegion::RegionSizeInBlocksSquared, false);
		TerrainHeightValues.SetNumUninitialized(FChunkRegion::RegionSizeInBlocksSquared, false);
		SFValues.SetNumUninitialized(FChunkRegion::RegionSizeInBlocksSquared, false);
		
	}

	TArray<float> ContinentalnessValues;
	TArray<float> ErosionValues;
	TArray<float> PVValues;
	TArray<float> SFValues;
	TArray<int16> TerrainHeightValues;
};