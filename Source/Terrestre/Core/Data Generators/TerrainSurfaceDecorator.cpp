#include "TerrainSurfaceDecorator.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"



void UTerrainSurfaceDecorator::GenerateTerrainSurfaceDecorations(TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& chunkBlocks, FVector chunkLocation, bool bDensitySignChange, TArray<int16>& heightMap)
{
	
	if (true) // Check if surface height could be somewhere in this chunk
	{
		chunkLocation /= AChunk::VoxelSize;
		//FBlockState grassBlock{ 2 };
		for (uint8 x = 0; x < AChunk::Size; x++)
		{
			//int32 worldX = chunkLocation.X + x;
			for (uint8 y = 0; y < AChunk::Size; y++)
			{
				int16 heightMapIndex = x + y * AChunk::Size;
				int16 localZsurface = heightMap[heightMapIndex] - chunkLocation.Z;
				check(localZsurface != 16);
				localZsurface--;
				
				if (localZsurface >= 0 && localZsurface < AChunk::Size)
				{
					int16 blockIndex = UChunkUtilityLib::LocalBlockPosToIndex({ x,y,localZsurface}); 
					*(chunkBlocks.GetData() + blockIndex) = FBlockState(3);
				}
			
		
			}
		}
	}
}