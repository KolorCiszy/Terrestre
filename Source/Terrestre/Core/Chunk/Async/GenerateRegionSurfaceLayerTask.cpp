#include "GenerateRegionSurfaceLayerTask.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Terrestre/Core/Data Generators/TerrainSurfaceDecorator.h"
#include "Logging/StructuredLog.h"

void FGenerateRegionSurfaceLayerTask::DoWork(EQueuedWorkPriority priority)
{
	AsyncPool(*URegionManager::Get()->ChunkRegionGenerationTP, [&]()
		{
			auto start = std::chrono::high_resolution_clock::now();
	if (!bShouldExit)
	{
		FChunkProtoRegion thisRegion = *(URegionManager::Get()->GetChunkRegionByID_WG(RegionID));
		
		if (UChunkUtilityLib::GetChunkManager()->TerrainSurfaceDecorator->GenerateTerrainSurfaceDecorations(thisRegion, RegionID))
		{
			RegionData = thisRegion;
			RegionData.CurrentGenStage = ERegionGenerationStages::SurfaceLayer;
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			UE_LOGFMT(LogTemp, Log, "Surface layer gen for region {0} took {1} ms", RegionID.ToString(), duration.count());
			URegionManager::Get()->RegionDataGenComplete(RegionID);
		}
		else
		{
			URegionManager::Get()->RegionDataGenFailed(RegionID);
		}
		
	}
	}, nullptr, priority);
	
	

}