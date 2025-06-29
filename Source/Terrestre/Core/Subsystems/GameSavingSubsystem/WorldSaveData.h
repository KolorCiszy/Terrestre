#pragma once

#include "CoreMinimal.h"
#include "WorldSaveData.generated.h"

UCLASS(BlueprintType)
class TERRESTRE_API UWorldSaveData : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FString WorldName;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	int64 WorldSeed;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FDateTime CreationDate;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FDateTime LastSaveDate;

	UFUNCTION(BlueprintPure)
	static int64 WorldSeedFromString(const FString& inString)
	{
 		int64 seed{};
		if(!inString.IsEmpty())
		{
			uint32 LeftBits = FCrc::MemCrc32(*inString, inString.Len() * sizeof(TCHAR));
			uint32 RightBits = FCrc::MemCrc32(*inString.Reverse(), inString.Len() * sizeof(TCHAR));
			seed = seed | LeftBits;
			seed <<= 32;
			seed = seed | RightBits;
		}
		return seed;
	}
};
FORCEINLINE
FArchive& operator<<(FArchive& ar, UWorldSaveData& data)
{
	ar << data.WorldName;
	ar << data.WorldSeed;
	ar << data.CreationDate;
	ar << data.LastSaveDate;
	return ar;
}
FORCEINLINE bool operator==(const UWorldSaveData& a, const UWorldSaveData& b)
{ 
	return a.WorldName == b.WorldName;
}