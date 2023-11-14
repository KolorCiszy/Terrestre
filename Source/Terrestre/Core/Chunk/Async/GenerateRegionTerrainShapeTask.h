#pragma once

#include "GenerateChunkRegionDataTask.h"

class FGenerateRegionTerrainShapeTask : public FGenerateChunkRegionDataTask
{
public:
	FGenerateRegionTerrainShapeTask(FIntVector regionID) : FGenerateChunkRegionDataTask(regionID) {};

	void DoWork(EQueuedWorkPriority priority) override;

};