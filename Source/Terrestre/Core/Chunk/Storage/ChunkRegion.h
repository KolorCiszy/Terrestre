#pragma once

#include "CoreMinimal.h"
#include "BlockPalette.h"
#include "Terrestre/Core/Chunk/Async/RegionGenerationStages.h"
#include "Terrestre/Core/Chunk/Chunk.h"
#include "ChunkData.h"

struct FChunkRegion;

struct FChunkProtoRegion
{
	FChunkProtoRegion() {};
	FChunkProtoRegion(const FChunkProtoRegion&) = delete;
	FChunkProtoRegion& operator=(const FChunkProtoRegion&) = delete;

	FChunkProtoRegion& operator=(FChunkRegion& packedData);
	FChunkProtoRegion(FChunkRegion& packedData);
	/* Data hash map linking chunk location to corresponding proto chunk data
	 * USED DURING WORLD GENERATION TASKS, DATA IS UNPACKED
	 */
	TMap<FVector, FProtoChunkData> ProtoChunkData;
};

struct FChunkRegion
{
	FChunkRegion() 
	{ 
		ChunkRefCount = 0;
		CurrentGenStage = ERegionGenerationStages::None;
		bIsBorderRegion = true;
		bIsLoaded = false;

	};
	FChunkRegion(const FChunkRegion& copy) = default;
	
	FChunkRegion& operator=(const FChunkRegion& copy) = default;
	
	FChunkRegion(FChunkRegion&& move)
	{
		ChunkRefCount = move.ChunkRefCount;
		ChunkData = MoveTemp(ChunkData);
		CurrentGenStage = move.CurrentGenStage;
	}
	FChunkRegion& operator=(FChunkRegion&& move)
	{
		ChunkRefCount = move.ChunkRefCount;
		ChunkData = MoveTemp(move.ChunkData);
		CurrentGenStage = move.CurrentGenStage;
		return *this;
	}
	FChunkRegion(FChunkProtoRegion& protoRegion)
	{
		for(auto& protoData : protoRegion.ProtoChunkData)
		{
			FChunkData TempChunkData = protoData.Value;
			ChunkData.Add(protoData.Key, MoveTemp(TempChunkData));
		}
		ChunkRefCount = 0;
	}
	FChunkRegion& operator=(FChunkProtoRegion& protoRegion)
	{
		for (auto& protoData : protoRegion.ProtoChunkData)
		{
			FChunkData TempChunkData = protoData.Value;
			ChunkData.Add(protoData.Key, MoveTemp(TempChunkData));
		}
		ChunkRefCount = 0;
		return *this;
	}
	/* Data hash map linking chunk location to corresponding block palette */
	TMap<FVector, FChunkData> ChunkData;
	/* How many chunks are referencing data from this region*/
	int32 ChunkRefCount;
	

	ERegionGenerationStages CurrentGenStage;

	bool bIsBorderRegion;

    bool bIsLoaded;

	void MarkPendingSave();

/*** STATIC MEMBERS ***/

	/* Amount of chunks in one region on every axis */
	static constexpr uint8 RegionSize = 8u;
	/* Amount of chunks in one region on every axis squared */
	static constexpr uint16 RegionSizeSquared = RegionSize * RegionSize;
	/* Amount of block in one region on one axis (length of an edge of region) */
	static constexpr uint16 RegionSizeInBlocks = RegionSize * AChunk::Size;
	/* Amount of block in one region on one axis (length of an edge of region) squared */
	static constexpr uint32 RegionSizeInBlocksSquared = RegionSizeInBlocks * RegionSizeInBlocks;

	static constexpr uint32 RegionVolumeInBlocks = RegionSizeInBlocks * RegionSizeInBlocksSquared;
	/* Amount of chunks in one region (volume of region in chunks) */
	static constexpr uint16 RegionVolume = RegionSize * RegionSize * RegionSize;
	/* The size of chunk region on every axis, scaled to unreal units */
	static inline FIntVector RegionSizeScaled{ RegionSize * AChunk::SizeScaled };
	

};
