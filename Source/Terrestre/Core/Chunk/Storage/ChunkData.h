#pragma once

#include "CoreMinimal.h"
#include "BlockPalette.h"
#include "FluidState.h"
#include "Terrestre/Core/Chunk/Chunk.h"

class FChunkData;

/* Contains a hash map of chunk data in its raw form (block states and fluid sates are unpacked) */
class FProtoChunkData
{
public:


	FProtoChunkData(const FProtoChunkData&) = delete;
	FProtoChunkData& operator=(const FProtoChunkData&) = delete;

	FProtoChunkData()
	{
		BlockStates.Init(FBlockState(), AChunk::Volume);
		FluidStates.Init(FFluidState(), AChunk::Volume);
	}
	FProtoChunkData(TArray<FBlockState, TInlineAllocator<AChunk::Volume>>&& blocks,
		TArray<FFluidState, TInlineAllocator<AChunk::Volume>>&& fluids)
	{
		BlockStates = blocks;
		FluidStates = fluids;
	};
	TArray<FBlockState, TInlineAllocator<AChunk::Volume>> BlockStates;
	TArray<FFluidState, TInlineAllocator<AChunk::Volume>> FluidStates;

	FProtoChunkData(FProtoChunkData&& move)
	{
		BlockStates = MoveTemp(move.BlockStates);
		FluidStates = MoveTemp(move.FluidStates);
	}
	FProtoChunkData& operator=(FProtoChunkData&& move)
	{
		BlockStates = MoveTemp(move.BlockStates);
		FluidStates = MoveTemp(move.FluidStates);
		return *this;
	}

	FProtoChunkData(const FChunkData& PackedData);
	
	FProtoChunkData& operator=(const FChunkData& PackedData);

};

class FChunkData
{
public:

	FBlockPalette BlockPalette;
	TArray<FFluidState, TInlineAllocator<AChunk::Volume>> FluidStates;

	FChunkData(TArray<FBlockState, TInlineAllocator<AChunk::Volume>>& blocks, 
				TArray<FFluidState, TInlineAllocator<AChunk::Volume>>&& fluids) 
	{
		BlockPalette = blocks;
		FluidStates = fluids;
	};
	FChunkData(FProtoChunkData& protoChunkData)
	{
		BlockPalette = protoChunkData.BlockStates;
		FluidStates = MoveTemp(protoChunkData.FluidStates);
	}


	FChunkData(FChunkData&& move)
	{
		BlockPalette = MoveTemp(move.BlockPalette);
		FluidStates = MoveTemp(move.FluidStates);
	}
	FChunkData(const FChunkData&) = default;
	FChunkData& operator=(const FChunkData&) = default;
	

	FChunkData& operator=(FChunkData&& move)
	{
		BlockPalette = MoveTemp(move.BlockPalette);
		FluidStates = MoveTemp(move.FluidStates);
		return *this;
	}
};
