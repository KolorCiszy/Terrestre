#pragma once

#include "CoreMinimal.h"
#include "Terrestre/Core/Chunk/Async/IRegionGen.h"
#include "ChunkRegion.h"
#include "RegionManager.generated.h"

class FGenerateChunkRegionDataTask;

UCLASS()
class URegionManager : public UObject, public IRegionGen
{
	GENERATED_BODY()

	friend class AChunkManager;
	
public:
	URegionManager();

	static URegionManager* Get();

	FChunkRegion* GetChunkRegionByID(FIntVector regionID);

	FChunkRegion* GetChunkRegionByID_WG(FIntVector regionID);

	const FBlockState GetBlockStateAtWorldBlockLoc_ReadOnly(FIntVector worldBlockLoc);
	
	/* Called from worker thread to inform the manager that the work is finished*/
	virtual void RegionDataGenComplete(FIntVector regionID) override;

	virtual void RegionDataGenFailed(FIntVector regionID) override;

	FQueuedThreadPool* ChunkRegionGenerationTP;
private:
	void BeginPlay(FVector centerChunk);

	void Tick(FVector centerChunk);

	void EndPlay();

	bool CreateRegionWithID(FIntVector regionID, bool bBorderRegion);

	bool RemoveRegionWithID(FIntVector regionID);

	void AsyncGenerateRegionData(FIntVector regionID);

	template<class T>
	FORCEINLINE void CreateAndStartRegionGenTask(FIntVector regionID, EQueuedWorkPriority priority)
	{
		static_assert(std::is_base_of<FGenerateChunkRegionDataTask, T>::value, "All region generation tasks must derive from FGenerateChunkRegionDataTask");
		T* newTask = new T(regionID);

		newTask->DoWork(priority);

		RegionGenerationTasks.Add(regionID, newTask);

		RegionsCurrentlyGenerating.Add(regionID);
	}
	/* Center chunk is the chunk the player is located in */
	void UpdateActiveRegionsIDs(FVector centerChunk);

	/* Holds all region data  */
	TMap<FIntVector, FChunkRegion> RegionsMap;

	TQueue<FIntVector> RegionsToGetDataFrom;

	/* FIntVector - region ID , bool - border region flag */
	TMap<FIntVector, bool> ActiveRegionsIDs; 

	TSet<FIntVector> RegionsToLoad;

	TSet<FIntVector> RegionsToUnload;

	TSet<FIntVector> RegionsCurrentlyGenerating;

	TMap<FIntVector, FGenerateChunkRegionDataTask*> RegionGenerationTasks;

	void GetRegionDataAndDeleteTask(FIntVector regionID);

	void CreateChunkRegionGenerationThreadPool();

	void UpdateRegionsToLoad();

	void UpdateRegionsToUnload();
	
	void LoadPendingRegions();

	FRWLock RWLock;

	FIntVector currentCenterRegionID;

};

