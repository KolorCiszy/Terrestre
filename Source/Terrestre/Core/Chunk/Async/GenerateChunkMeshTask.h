#pragma once
#include "CoreMinimal.h"
#include "Async/AsyncWork.h"
#include "RealtimeMeshSimple.h"
#include "Terrestre/Core/Chunk/Chunk.h"


enum class EDirections;

using FMeshData = FRealtimeMeshSimpleMeshData;

struct FChunkHelper;
struct FBlockState;
class FBlockPalette;
class AChunk;




class FGenerateChunkMeshTask : public FNonAbandonableTask
{
	friend class FAsyncTask<FGenerateChunkMeshTask>;

public: 
	FGenerateChunkMeshTask() = delete;
	FGenerateChunkMeshTask(TObjectPtr<AChunk> chunkOwner) : chunk{ chunkOwner }, meshData{ MakeUnique<FMeshData>()} 
	{
		ResetData();
	};
	//* the chunk that requested generation
	TObjectPtr<AChunk> chunk;
	//* mesh data this task will generate
	TUniquePtr<FMeshData> meshData;


	
	void ResetData();
private: 
	
	void DoWork();
	
	bool IsVisibleFace(FIntVector localPos, EDirections direction);
	
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
	/*
	* tlv - top left vertex
	* blv - bottom left vertex
	* brv - bottom right vertex
	* trv - top right vertex
	* norm - quad normal
	*/
	void CreateQuad(FVector tlv, FVector trv, FVector blv, FVector brv, FVector norm, const FBlockState& block);


	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FGenerateChunkMeshTask, STATGROUP_ThreadPoolAsyncTasks); }
	
};

