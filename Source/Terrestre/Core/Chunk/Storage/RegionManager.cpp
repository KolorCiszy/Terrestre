#include "RegionManager.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Terrestre/Core/Chunk/Async/GenerateRegionTerrainShapeTask.h"
#include "Terrestre/Core/Chunk/Async/GenerateRegionSurfaceLayerTask.h"

URegionManager::URegionManager()
{
	
}

URegionManager* URegionManager::Get()
{
	return UChunkUtilityLib::GetChunkManager()->RegionManager;
}
void URegionManager::CreateChunkRegionGenerationThreadPool()
{
	int32 threadsToCreate = FPlatformMisc::NumberOfCoresIncludingHyperthreads() - 4;
	if (threadsToCreate <= 0)
	{
		return;
	}
	ChunkRegionGenerationTP = FQueuedThreadPool::Allocate();
	if (ChunkRegionGenerationTP)
	{
		ChunkRegionGenerationTP->Create(threadsToCreate, 32 * 1024, EThreadPriority::TPri_Normal, TEXT("Chunk Region Data Generation Thread Pool"));
	}
}


FChunkRegion* URegionManager::GetChunkRegionByID(FIntVector regionID)
{
	FRWScopeLock lock(RWLock, FRWScopeLockType::SLT_ReadOnly);
	if (FChunkRegion* region = RegionsMap.Find(regionID))
	{
		if (region->bIsLoaded && region->CurrentGenStage != ERegionGenerationStages::TerrainShape)
		{
			return region;
		}
	}
	return nullptr;
}
FChunkRegion* URegionManager::GetChunkRegionByID_WG(FIntVector regionID)
{
	FRWScopeLock lock(RWLock, FRWScopeLockType::SLT_ReadOnly);
	if (auto region = RegionsMap.Find(regionID))
	{
		if (region->bIsLoaded)
		{
			return region;
		}
	}
	return nullptr;
}
const FBlockState URegionManager::GetBlockStateAtWorldBlockLoc_ReadOnly(FIntVector worldBlockLoc)
{
	FVector ChunkLoc = UChunkUtilityLib::BlockPosToChunkWorldLocation(worldBlockLoc);
	FIntVector RegionID = UChunkUtilityLib::GetRegionID(ChunkLoc);
	FRWScopeLock Lock(RWLock, FRWScopeLockType::SLT_ReadOnly);
	FChunkRegion* Region = RegionsMap.Find(RegionID);
	if (Region)
		{
			if (Region->bIsLoaded)
			{
				int32 ChunkIndex = UChunkUtilityLib::LocalBlockPosToIndex(UChunkUtilityLib::BlockPosToLocalBlockPos(worldBlockLoc));
				return Region->ChunkData[ChunkLoc].BlockPalette.GetBlockAtIndex(ChunkIndex);
			}
		}
	return FBlockState{};
}
bool URegionManager::CreateRegionWithID(FIntVector regionID, bool bBorderRegion)
{
	if (RWLock.TryReadLock())
	{
		FChunkRegion* region = RegionsMap.Find(regionID);
		RWLock.ReadUnlock();
		if (region)
		{
			return false;
		}
		RWLock.WriteLock();
		
		FChunkRegion* createdRegion = &RegionsMap.Add({ regionID, FChunkRegion() });
		createdRegion->bIsBorderRegion = bBorderRegion;
		createdRegion->bIsLoaded = false;

		RWLock.WriteUnlock();
		return true;
	}
	return false;
}

bool URegionManager::RemoveRegionWithID(FIntVector regionID)
{
	FRWScopeLock lock(RWLock, FRWScopeLockType::SLT_Write);
	RegionsMap.Remove(regionID);
	return true;
}

void URegionManager::AsyncGenerateRegionData(FIntVector regionID)
{
	
	RWLock.ReadLock();
	EQueuedWorkPriority priority = RegionsMap[regionID].bIsBorderRegion ? EQueuedWorkPriority::Low : EQueuedWorkPriority::Highest;
	ERegionGenerationStages genStage = RegionsMap[regionID].CurrentGenStage;
	RWLock.ReadUnlock();
	
	switch (genStage)
	{
	case ERegionGenerationStages::None: CreateAndStartRegionGenTask<FGenerateRegionTerrainShapeTask>(regionID, priority);
		break;
	case ERegionGenerationStages::TerrainShape: CreateAndStartRegionGenTask<FGenerateRegionSurfaceLayerTask>(regionID, priority);
		break;
	default: checkf(false, TEXT("Wrong gen stage creating task with id %s"), *regionID.ToString())
		break;
	}
	
}


void URegionManager::RegionDataGenComplete(FIntVector regionID)
{
	RegionsToGetDataFrom.Enqueue(regionID);
}
void URegionManager::RegionDataGenFailed(FIntVector regionID)
{
	AsyncTask(ENamedThreads::GameThread,
		[regionID, this]()
		{
			auto task = *RegionGenerationTasks.Find(regionID);
			if (task)
			{
				//task->EnsureCompletion(false, true);
				//task->Reschedule(ChunkRegionGenerationTP, EQueuedWorkPriority::Lowest);
			}
		});
}
void URegionManager::GetRegionDataAndDeleteTask(FIntVector regionID)
{
	auto AsyncTask = RegionGenerationTasks[regionID];
	if (AsyncTask)
	{
		FChunkRegion* region{};
		ERegionGenerationStages GenStage;
			
		RWLock.WriteLock();
		region = RegionsMap.Find(regionID);

		if (region)
		{
			*region = MoveTemp(AsyncTask->RegionData);
		}
		else
		{
			RegionsMap.Add(regionID);
			RegionsMap[regionID] = MoveTemp(AsyncTask->RegionData);
		}
		region->bIsLoaded = true;
		GenStage = region->CurrentGenStage;

		RWLock.WriteUnlock();

		RegionsToLoad.Remove(regionID);
		RegionsCurrentlyGenerating.Remove(regionID);
		delete AsyncTask;
		RegionGenerationTasks.Remove(regionID);
		RegionsToGetDataFrom.Dequeue(regionID);
		
	}
}
void URegionManager::UpdateActiveRegionsIDs(FVector centerChunk)
{
	ActiveRegionsIDs.Reset();
	FIntVector currentRegionID = UChunkUtilityLib::GetRegionID(centerChunk);
	uint8 RegionLoadDistance = UChunkUtilityLib::GetChunkManager()->RegionLoadDistance;
	for (int8 x = -RegionLoadDistance; x <= RegionLoadDistance; x++)
	{
		for (int8 y = -RegionLoadDistance; y <= RegionLoadDistance; y++)
		{
			for (int8 z = -RegionLoadDistance; z <= RegionLoadDistance; z++)
			{
				FIntVector regionID = currentRegionID + FIntVector{ x,y,z };
				bool bBorderRegion = false;
				if (FMath::Abs(x) == RegionLoadDistance ||
					FMath::Abs(y) == RegionLoadDistance || 
					FMath::Abs(z) == RegionLoadDistance)
				{
					bBorderRegion = true;
				}
				ActiveRegionsIDs.Add({ regionID, bBorderRegion });
			}
		}
	}
}
void URegionManager::UpdateRegionsToLoad()
{
	FRWScopeLock RWSLock(RWLock, FRWScopeLockType::SLT_ReadOnly);
	for (auto& region : RegionsMap)
	{
		bool bIsBeingGenerated = RegionsCurrentlyGenerating.Contains(region.Key);
		if (!region.Value.bIsLoaded && !bIsBeingGenerated)
		{
			RegionsToLoad.Add(region.Key);
		}
		else 
		{
			
			if (!region.Value.bIsBorderRegion && !bIsBeingGenerated && region.Value.CurrentGenStage == ERegionGenerationStages::TerrainShape)
			{
				FIntVector UpRegionID{ region.Key.X, region.Key.Y, region.Key.Z + 1 };
				FIntVector DownRegionID{ region.Key.X, region.Key.Y, region.Key.Z - 1 };
				auto UpRegion = RegionsMap.Find(UpRegionID);
				auto DownRegion = RegionsMap.Find(DownRegionID);
				if (UpRegion->bIsLoaded && DownRegion->bIsLoaded)
				{
					RegionsToLoad.Add(region.Key);
				}
			}
		}
	}
}
void URegionManager::LoadPendingRegions()
{
	for (auto& region : RegionsToLoad)
	{
		// TODO chech if can load region from file, otherwise;
		if (!RegionsCurrentlyGenerating.Contains(region))
		{
			AsyncGenerateRegionData(region);
		}
		
	}
}
void URegionManager::UpdateRegionsToUnload()
{
	FRWScopeLock RWSLock(RWLock, FRWScopeLockType::SLT_ReadOnly);
	for (auto& region : RegionsMap)
	{
		if (!ActiveRegionsIDs.Contains(region.Key))
		{
			RegionsToUnload.Add(region.Key);
		}
	}
}
void URegionManager::BeginPlay(FVector centerChunk)
{
	CreateChunkRegionGenerationThreadPool();

	UpdateActiveRegionsIDs(centerChunk);
	currentCenterRegionID = UChunkUtilityLib::GetRegionID(centerChunk);

	for (auto& IDs : ActiveRegionsIDs)
	{
		CreateRegionWithID(IDs.Key, IDs.Value);
	}
	UpdateRegionsToLoad();

	LoadPendingRegions();
}

void URegionManager::Tick(FVector centerChunk)
{
	FIntVector currentRegionID = UChunkUtilityLib::GetRegionID(centerChunk);
	if (FIntVector* regionID = RegionsToGetDataFrom.Peek())
	{
		GetRegionDataAndDeleteTask(*regionID);
		UpdateRegionsToLoad();
		LoadPendingRegions();
	}
	
	else if (currentCenterRegionID != currentRegionID)
	{
		
		UpdateActiveRegionsIDs(centerChunk);


		for (auto& region : ActiveRegionsIDs)
		{

			if (!RegionsMap.Contains(region.Key))
			{
				CreateRegionWithID(region.Key, region.Value);
			}
			RegionsMap[region.Key].bIsBorderRegion = region.Value;
		}
		UpdateRegionsToLoad();
		LoadPendingRegions();

		currentCenterRegionID = currentRegionID;
			
	}
	
	/*
	for (auto& task : RegionGenerationTasks)
	{
		if(task.Value->IsIdle())
		{
			task.Value->Reschedule(ChunkRegionGenerationTP);
		}
	}
	*/
	
	
}
void URegionManager::EndPlay()
{
	while(!RegionGenerationTasks.IsEmpty())
	{
		FPlatformProcess::Sleep(0.05f);
		if (FIntVector* regionID = RegionsToGetDataFrom.Peek())
		{
			GetRegionDataAndDeleteTask(*regionID);
		}
	}
	
	RegionGenerationTasks.Empty();
	ChunkRegionGenerationTP->Destroy();
	delete ChunkRegionGenerationTP;
}