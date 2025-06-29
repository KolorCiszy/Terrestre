#pragma once
#include "CoreMinimal.h"
#include "ChunkConstants.h"

class FBlockOpacityData
{
public:
	FBlockOpacityData() 
	{ 
		data.SetNumUninitialized(FChunkConstants::Volume); 
		IsReady = false;
	};
	FBitReference operator[](int32 index)
	{
		return data[index];
	}
	void SetBlockOpacityAtIndex(int32 index, bool bIsOpaque)
	{
		data[index] = bIsOpaque;
	}
	bool IsReady;
	void MakeFullyOpaque()
	{
		data.SetRange(0,FChunkConstants::Volume, true);
	}
private:
	TBitArray<TInlineAllocator<FChunkConstants::Volume>> data;
};