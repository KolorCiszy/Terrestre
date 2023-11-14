#pragma once

#include "CoreMinimal.h"
#include "ChunkHelper.generated.h"

//Stores flags to know if a face has already been checked
USTRUCT()
struct FChunkHelper
{
	GENERATED_BODY()
public:
	TArray<bool> visitedXN; 
	TArray<bool> visitedXP; 
	TArray<bool> visitedZN; 
	TArray<bool> visitedZP; 
	TArray<bool> visitedYN; 
	TArray<bool> visitedYP; 

	FORCEINLINE
	void SetSize(int32 chunkVolume)
	{
		visitedXN.SetNum(chunkVolume);
		visitedXP.SetNum(chunkVolume);
		visitedYN.SetNum(chunkVolume);
		visitedYP.SetNum(chunkVolume);
		visitedZN.SetNum(chunkVolume);
		visitedZP.SetNum(chunkVolume);
	}
};

