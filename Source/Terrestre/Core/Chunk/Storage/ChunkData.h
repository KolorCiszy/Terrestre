#pragma once

#include "CoreMinimal.h"
#include "BlockPalette.h"
#include "FluidState.h"
#include "Terrestre/Core/Chunk/Chunk.h"

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

	FChunkData(FChunkData&& move)
	{
		BlockPalette = MoveTemp(move.BlockPalette);
		FluidStates = MoveTemp(move.FluidStates);
	}
	FChunkData& operator=(const FChunkData&) = delete;

	FChunkData& operator=(FChunkData&& move)
	{
		BlockPalette = MoveTemp(move.BlockPalette);
		FluidStates = MoveTemp(move.FluidStates);
		return *this;
	}
};