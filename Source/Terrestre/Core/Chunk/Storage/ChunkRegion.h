#pragma once

#include "CoreMinimal.h"
#include "BlockPalette.h"
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
		bIsBorderRegion = true;
		bIsLoaded = false;
	};
	FChunkRegion(const FChunkRegion& copy) = default;
	
	FChunkRegion& operator=(const FChunkRegion& copy) = default;
	
	FChunkRegion(FChunkRegion&& move)
	{
		ChunkRefCount = move.ChunkRefCount;
		ChunkData = MoveTemp(ChunkData);
		
	}
	FChunkRegion& operator=(FChunkRegion&& move)
	{
		ChunkRefCount = move.ChunkRefCount;
		ChunkData = MoveTemp(move.ChunkData);
	
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
	
	FIntVector ID;


	bool bIsBorderRegion;

    bool bIsLoaded;

	void MarkPendingSave();

/*** STATIC MEMBERS ***/

	
	
	
};
FORCEINLINE FArchive& operator<<(FArchive& ar, FChunkRegion& region)
{
	ar << region.ChunkData;
	return ar;
}
