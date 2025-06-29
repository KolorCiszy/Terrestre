#pragma once

#include "CoreMinimal.h"
#include "ChunkRegion.h"
#include "RegionManager.generated.h"

class FGenerateChunkRegionDataTask;

UCLASS()
class ARegionManager : public AActor
{
	GENERATED_BODY()

	
	
public:
	ARegionManager();


	FChunkRegion* GetChunkRegionByID(FIntVector regionID) {};

	

	const FBlockState GetBlockStateAtWorldBlockLoc_ReadOnly(FIntVector worldBlockLoc) {};

protected:

	void CreateRegionWithID(FIntVector regionID, bool bBorderRegion) {};

	bool RemoveRegionWithID(FIntVector regionID) {};

	void AsyncGenerateRegionData(FIntVector regionID) {};


	/* Center chunk is the chunk the player is located in */
	void UpdateActiveRegionsIDs(FVector centerChunk) {};

	/* Holds all region data  */
	TMap<FIntVector, FChunkRegion> RegionsMap;
	/* Read write lock for accesing region map data */
	FRWLock RWLock;
	/* FIntVector - region ID , bool - border region flag */
	TMap<FIntVector, bool> ActiveRegionsIDs; 

	TSet<FIntVector> RegionsToLoad;

	TSet<FIntVector> RegionsToUnload;

	TMap<FIntVector, TSharedFuture<FChunkRegion>> PendingRegions;

	void UpdateRegionsToLoad() {};

	void UpdateRegionsToUnload() {};
	
	void LoadPendingRegions() {};



	FIntVector currentCenterRegionID;

};

