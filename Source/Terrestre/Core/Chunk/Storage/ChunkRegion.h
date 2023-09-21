#pragma once

#include "CoreMinimal.h"
#include "BlockPalette.h"
#include "Terrestre/Core/Chunk/Chunk.h"
#include "ChunkData.h"



struct FChunkRegion
{

	FChunkRegion() 
	{ 
		RefCount = 0;
	};
	FChunkRegion(const FChunkRegion&) = delete;
	FChunkRegion& operator=(const FChunkRegion&) = delete;
	FChunkRegion(FChunkRegion&& move)
	{
		RefCount = move.RefCount;
		ChunkData = MoveTemp(ChunkData);
	}
	FChunkRegion& operator=(FChunkRegion&& move)
	{
		RefCount = move.RefCount;
		ChunkData = MoveTemp(move.ChunkData);
		return *this;
	}
	/* Data hash map linking chunk location to corresponding block palette */
	TMap<FVector, FChunkData> ChunkData;
	/* How many chunks are referencing data from this region*/
	int32 RefCount;
	/* Amount of chunks in one region on every axis */
	static constexpr uint8 RegionSize = 16;
	/* Amount of chunks in one region (volume of region in chunks) */
	static constexpr uint16 RegionVolume = RegionSize * RegionSize * RegionSize;
	/* The size of chunk region on every axis, scaled to unreal units */
	static inline FIntVector RegionSizeScaled{ RegionSize * AChunk::SizeScaled };
	
	void MarkPendingSave();
};
