#pragma once
#include "CoreMinimal.h"
#include "RealtimeMeshSimple.h"
#include "Terrestre/Core/Chunk/Storage/BlockState.h"
#include "Terrestre/Core/Chunk/Storage/FluidState.h"
#include "Terrestre/Core/Chunk/Chunk.h"




using FMeshData = FRealtimeMeshStreamSet;

struct FChunkHelper;

class FBlockPalette;


class FGenerateChunkMeshTask : public FNonAbandonableTask
{
	friend class FAsyncTask<FGenerateChunkMeshTask>;

public: 
	FGenerateChunkMeshTask();
	~FGenerateChunkMeshTask();
	FGenerateChunkMeshTask(TObjectPtr<AChunk> chunkOwner);
	
	//* the chunk that owns this task generation
	TObjectPtr<AChunk> chunk;

	/* mesh data for blocks this task will generate */
	TUniquePtr<FMeshData> blockStateMeshData;
	
	/* mesh data for blocks this task will generate */
	TUniquePtr<FMeshData> fluidStateMeshData;


	
	void ResetData();
private: 
	
	void DoWork();
	
	void GenerateBlockStateMesh();

	void GenerateWaterMesh();

	bool IsVisibleFace(FIntVector localPos, EDirections direction);
	
	bool HasWater(FIntVector localPos, EDirections direction);

	bool forwardChunkDataValid;
	bool backwardChunkDataValid;
	bool rightChunkDataValid;
	bool leftChunkDataValid;
	bool upChunkDataValid;
	bool downChunkDataValid;

	TArray<FBlockState, TInlineAllocator<AChunk::Volume>> uncompressedBlocks{};
	TArray<FBlockState, TInlineAllocator<AChunk::Volume>> uncompressedBlocksF{};
	TArray<FBlockState, TInlineAllocator<AChunk::Volume>> uncompressedBlocksB{};
	TArray<FBlockState, TInlineAllocator<AChunk::Volume>> uncompressedBlocksL{};
	TArray<FBlockState, TInlineAllocator<AChunk::Volume>> uncompressedBlocksR{};
	TArray<FBlockState, TInlineAllocator<AChunk::Volume>> uncompressedBlocksU{};
	TArray<FBlockState, TInlineAllocator<AChunk::Volume>> uncompressedBlocksD{};

	TArray<FFluidState, TInlineAllocator<AChunk::Volume>> fluidStates;

	TArray<FFluidState, TInlineAllocator<AChunk::Volume>> fluidStatesU;
	TArray<FFluidState, TInlineAllocator<AChunk::Volume>> fluidStatesD;
	TArray<FFluidState, TInlineAllocator<AChunk::Volume>> fluidStatesL;
	TArray<FFluidState, TInlineAllocator<AChunk::Volume>> fluidStatesR;
	TArray<FFluidState, TInlineAllocator<AChunk::Volume>> fluidStatesF;
	TArray<FFluidState, TInlineAllocator<AChunk::Volume>> fluidStatesB;


	/*
	* tlv - top left vertex
	* blv - bottom left vertex
	* brv - bottom right vertex
	* trv - top right vertex
	* norm - quad normal
	*/
	void CreateQuad(TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1, uint16>* MeshBuilder, FVector tlv, FVector trv, FVector blv, FVector brv, FVector norm, const FBlockState& block);

	void CreateQuad(TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1, uint16>* MeshBuilder, FVector tlv, FVector trv, FVector blv, FVector brv, FVector norm, const FFluidState& fluid);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FGenerateChunkMeshTask, STATGROUP_ThreadPoolAsyncTasks); }

	void ClearMeshData(FMeshData& meshdata)
	{
		meshdata.Empty();
	}
};



