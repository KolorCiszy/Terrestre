#pragma once

#include "GenerateChunkRegionDataTask.h"

class FGenerateRegionSurfaceLayerTask : public FGenerateChunkRegionDataTask
{
public:
	FGenerateRegionSurfaceLayerTask(FIntVector regionID) : FGenerateChunkRegionDataTask{ regionID } {};

	void DoWork(EQueuedWorkPriority priority);

};



