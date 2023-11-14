#pragma once
#include "CoreMinimal.h"
#include "Async/AsyncWork.h"
#include "IRegionGen.h"
#include "Terrestre/Core/Chunk/Storage/ChunkRegion.h"


/* Base class for chunk region generation async tasks */
class FGenerateChunkRegionDataTask : public IRegionGen
{

public:
	FGenerateChunkRegionDataTask() = delete;
	FGenerateChunkRegionDataTask(FIntVector ID) : bShouldExit{ false }, RegionID{ ID } {};

	FChunkRegion RegionData;

	virtual void DoWork(EQueuedWorkPriority priority) = 0;

	std::atomic<bool> bShouldExit;

	virtual ~FGenerateChunkRegionDataTask() {};
protected:
	const FIntVector RegionID;
	
};