#pragma once

#include "CoreMinimal.h"

class IRegionGen
{
public:
	virtual void RegionDataGenComplete(FIntVector regionID) {};

	virtual void RegionDataGenFailed(FIntVector regionID) {};
};