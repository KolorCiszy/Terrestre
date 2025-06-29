#pragma once

#include "CoreMinimal.h"
#include "Terrestre/Core/Chunk/ChunkConstants.h"

struct FChunkGenerationData
{
	FChunkGenerationData()
	{
		ContinentalnessValues.SetNumUninitialized(FChunkConstants::SizeSquared, false);
		ErosionValues.SetNumUninitialized(FChunkConstants::SizeSquared, false);
		PVValues.SetNumUninitialized(FChunkConstants::SizeSquared, false);
		TerrainHeightValues.SetNumUninitialized(FChunkConstants::SizeSquared, false);
		SFValues.SetNumUninitialized(FChunkConstants::SizeSquared, false);
		
	}

	TArray<float> ContinentalnessValues;
	TArray<float> ErosionValues;
	TArray<float> PVValues;
	TArray<float> SFValues;
	TArray<int16> TerrainHeightValues;
};