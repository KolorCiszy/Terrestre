#pragma once

#include "CoreMinimal.h"
#include "BlockPalette.h"
#include "FluidState.h"
#include "Terrestre/Core/Chunk/ChunkConstants.h"
#include "Terrestre/Core/Chunk/ChunkGenStage.h"

class FChunkData;

/* Contains a hash map of chunk data in its raw form (block states and fluid sates are unpacked) */
class FProtoChunkData
{
public:

	TArray<FBlockState, TInlineAllocator<FChunkConstants::Volume>> BlockStates;
	TArray<FFluidState, TInlineAllocator<FChunkConstants::Volume>> FluidStates;
	EChunkGenStage GenStage;



	FProtoChunkData(const FProtoChunkData&) = delete;
	FProtoChunkData& operator=(const FProtoChunkData&) = delete;

	FProtoChunkData()
	{
		BlockStates.Init(FBlockState(), FChunkConstants::Volume);
		FluidStates.Init(FFluidState(), FChunkConstants::Volume);
		GenStage = EChunkGenStage::Empty;
	}
	FProtoChunkData(TArray<FBlockState, TInlineAllocator<FChunkConstants::Volume>>&& blocks,
					TArray<FFluidState, TInlineAllocator<FChunkConstants::Volume>>&& fluids,
					EChunkGenStage genStage)
	{
		BlockStates = blocks;
		FluidStates = fluids;
		GenStage = genStage;
	};
	

	FProtoChunkData(FProtoChunkData&& move)
	{
		BlockStates = MoveTemp(move.BlockStates);
		FluidStates = MoveTemp(move.FluidStates);
		GenStage = move.GenStage;
	}
	FProtoChunkData& operator=(FProtoChunkData&& move)
	{
		BlockStates = MoveTemp(move.BlockStates);
		FluidStates = MoveTemp(move.FluidStates);
		GenStage = move.GenStage;
		return *this;
	}

	FProtoChunkData(const FChunkData& PackedData);
	
	FProtoChunkData& operator=(const FChunkData& PackedData);

};

class FChunkData
{
public:

	FBlockPalette BlockPalette;
	TArray<FFluidState, TInlineAllocator<FChunkConstants::Volume>> FluidStates;
	EChunkGenStage GenStage;

	FChunkData() {};
	FChunkData(TArray<FBlockState, TInlineAllocator<FChunkConstants::Volume>>& blocks,
				TArray<FFluidState, TInlineAllocator<FChunkConstants::Volume>>&& fluids,
				EChunkGenStage genStage) 
	{
		BlockPalette = blocks;
		FluidStates = fluids;
		GenStage = genStage;
	};
	FChunkData(FProtoChunkData& protoChunkData)
	{
		BlockPalette = protoChunkData.BlockStates;
		FluidStates = MoveTemp(protoChunkData.FluidStates);
		GenStage = protoChunkData.GenStage;
	}


	FChunkData(FChunkData&& move)
	{
		BlockPalette = MoveTemp(move.BlockPalette);
		FluidStates = MoveTemp(move.FluidStates);
		GenStage = move.GenStage;
	}
	FChunkData(const FChunkData&) = default;
	FChunkData& operator=(const FChunkData&) = default;
	

	FChunkData& operator=(FChunkData&& move)
	{
		BlockPalette = MoveTemp(move.BlockPalette);
		FluidStates = MoveTemp(move.FluidStates);
		GenStage = move.GenStage;
		return *this;
	}
	
};
FORCEINLINE FArchive& operator<<(FArchive& ar, FChunkData& cd)
{
	ar << cd.BlockPalette;
	ar << cd.FluidStates;
	ar << cd.GenStage;
	return ar;
}
