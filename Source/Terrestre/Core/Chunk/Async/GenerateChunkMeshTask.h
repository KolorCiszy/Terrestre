#pragma once
#include "CoreMinimal.h"
#include "Terrestre/Core/Chunk/Storage/BlockState.h"
#include "Terrestre/Core/Chunk/Storage/FluidState.h"
#include "Terrestre/Core/Chunk/Chunk.h"
#include "Terrestre/Core/Chunk/ChunkConstants.h"
#include "Terrestre/Core/Chunk/Misc/Directions.h"
#include <Runtime/GeometryFramework/Public/Components/DynamicMeshComponent.h>

struct FChunkHelper;

class FBlockPalette;

class FGenerateChunkMeshTask : public FNonAbandonableTask
{
	friend class FAsyncTask<FGenerateChunkMeshTask>;
	friend class AChunk;
public: 
	FGenerateChunkMeshTask() = delete;
	~FGenerateChunkMeshTask();
	FGenerateChunkMeshTask(AChunk* chunkOwner);
	
	//* the chunk that owns this task
	TObjectPtr<AChunk> chunk;

	TSharedPtr<FDynamicMesh3, ESPMode::NotThreadSafe> BlockDynamicMeshData;

	TSharedPtr<FDynamicMesh3, ESPMode::NotThreadSafe> FluidDynamicMeshData;
private: 
	
	void DoWork();
	
	void GenerateBlockStateMesh();

	void GenerateWaterMesh();

	bool IsVisibleFace(FIntVector localPos, EDirections direction);
	
	bool HasWater(FIntVector localPos, EDirections direction);

	TMap<EDirections, FBlockOpacityData> ChunkOpacityCache;

	TMap < EDirections, TArray<FFluidState, TInlineAllocator<FChunkConstants::Volume>>> FluidStateCache;

	TArray<FBlockState, TInlineAllocator<FChunkConstants::Volume>> UnpackedBlocks;

	TArray<FFluidState, TInlineAllocator<FChunkConstants::Volume>> FluidStatesCopy;

	/*
	* tlv - top left vertex
	* blv - bottom left vertex
	* brv - bottom right vertex
	* trv - top right vertex
	* norm - quad normal
	*/
	void CreateQuad(FVector tlv, FVector trv, FVector blv, FVector brv, FVector norm, const FBlockState& block);

	void CreateQuad(FVector tlv, FVector trv, FVector blv, FVector brv, FVector norm, const FFluidState& fluid);

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FGenerateChunkMeshTask, STATGROUP_ThreadPoolAsyncTasks); }

};



