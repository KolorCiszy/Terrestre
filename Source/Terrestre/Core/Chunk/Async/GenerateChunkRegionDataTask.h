#pragma once
#include "CoreMinimal.h"
#include "Async/AsyncWork.h"
#include "Terrestre/Core/Chunk/Storage/ChunkRegion.h"

class FGenerateChunkRegionDataTask : public FNonAbandonableTask
{
	friend class FAsyncTask<FGenerateChunkRegionDataTask>;
	friend class AChunkManager;

public:
	FGenerateChunkRegionDataTask() = delete;
	FGenerateChunkRegionDataTask(FIntVector ID) : regionID{ ID } {}
private:
	FIntVector regionID;
	FChunkRegion newRegion;
	void DoWork();

	
	
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FGenerateChunkRegionDataTask, STATGROUP_ThreadPoolAsyncTasks); }
};