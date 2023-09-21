#include "GenerateChunkRegionDataTask.h"
#include "Terrestre/Core/Data Generators/TerrainShaper.h"
#include "Terrestre/Core/Data Generators/TerrainSurfaceDecorator.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"

void FGenerateChunkRegionDataTask::DoWork()
{
	
	newRegion.ChunkData.Reserve(FChunkRegion::RegionVolume);
	for (uint8 x = 0; x < FChunkRegion::RegionSize; x++)
	{
		for (uint8 y = 0; y < FChunkRegion::RegionSize; y++)
		{
			TArray<int16> heightMap{};
			heightMap.Init(INT16_MAX, AChunk::SizeSquared);
			for (uint8 z = 0; z < FChunkRegion::RegionSize; z++)
			{
				FVector chunkLocation = FVector((regionID * FChunkRegion::RegionSizeScaled) + (FIntVector{ x,y,z } *FIntVector(AChunk::SizeScaled)));
				TArray<FBlockState, TInlineAllocator<AChunk::Volume>> blocks{};
				TArray<FFluidState, TInlineAllocator<AChunk::Volume>> fluidStates{};
				blocks.SetNumUninitialized(AChunk::Volume);
				fluidStates.Init(FFluidState{ UINT8_MAX, 0 }, AChunk::Volume);

				bool bSignChange = false;
				UTerrainShaper::GenerateTerrainShape(blocks, fluidStates, chunkLocation, bSignChange, heightMap);
				//UTerrainSurfaceDecorator::GenerateTerrainSurfaceDecorations(blocks, chunkLocation, bSignChange, heightMap);
				//FBlockPalette palette = blocks;

				FChunkData chunkData{ blocks, MoveTemp(fluidStates) };
				
				newRegion.ChunkData.Add(MoveTemp(chunkLocation), MoveTemp(chunkData));
			}
		}
	}
	UChunkUtilityLib::GetChunkManager()->RegionDataGenerationComplete(regionID);
}