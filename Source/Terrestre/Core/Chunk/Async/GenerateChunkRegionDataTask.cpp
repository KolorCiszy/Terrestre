#include "GenerateChunkRegionDataTask.h"
#include "Terrestre/Core/Data Generators/TerrainShaper.h"
#include "Terrestre/Core/Data Generators/TerrainSurfaceDecorator.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"

void FGenerateChunkRegionDataTask::DoWork()
{
	
	newRegion.Data.Reserve(FChunkRegion::RegionVolume);
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
				
				bool bSignChange = false;
				UTerrainShaper::GenerateTerrainShape(blocks, chunkLocation, bSignChange, heightMap);
				//UTerrainSurfaceDecorator::GenerateTerrainSurfaceDecorations(blocks, chunkLocation, bSignChange, heightMap);
				FBlockPalette palette = blocks;
				newRegion.Data.Add(MoveTemp(chunkLocation), MoveTemp(palette));
			}
		}
	}
	UChunkUtilityLib::GetChunkManager()->RegionDataGenerationComplete(regionID);
}