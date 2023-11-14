#include "GenerateRegionTerrainShapeTask.h"
#include "Terrestre/Core/Data Generators/RegionGenerationData.h"
#include "Terrestre/Core/Data Generators/TerrainShaper.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Terrestre/Core/Chunk/Storage/RegionManager.h"
#include "Logging/StructuredLog.h"



void FGenerateRegionTerrainShapeTask::DoWork(EQueuedWorkPriority priority)
{
	AsyncPool(*URegionManager::Get()->ChunkRegionGenerationTP, [&]()
		{
			auto start1 = std::chrono::high_resolution_clock::now();
	FChunkProtoRegion ProtoRegion{};
	ProtoRegion.ProtoChunkData.Reserve(FChunkRegion::RegionVolume);

	FRegionGenerationData GenerationData{};


	UChunkUtilityLib::GetTerrainShaper()->GenerateRegionInitialData(GenerationData, RegionID);
	auto end1 = std::chrono::high_resolution_clock::now();


	auto start2 = std::chrono::high_resolution_clock::now();
	UChunkUtilityLib::GetTerrainShaper()->GenerateTerrainShape(ProtoRegion, GenerationData, RegionID);

	RegionData = ProtoRegion;
	RegionData.CurrentGenStage = ERegionGenerationStages::TerrainShape;
	auto end2 = std::chrono::high_resolution_clock::now();
	auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
	auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
	auto durSum = duration1.count() + duration2.count();
	UE_LOGFMT(LogTemp, Log, "Terrain shape gen for region {0} took {1} ms, first part: {2}, second part: {3}",
		RegionID.ToString(), durSum, duration1.count(), duration2.count());


	URegionManager::Get()->RegionDataGenComplete(RegionID);
	
		}, nullptr, priority);
	
}