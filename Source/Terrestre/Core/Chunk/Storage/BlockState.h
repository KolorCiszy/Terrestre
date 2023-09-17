#pragma once

#include "CoreMinimal.h"
#include "BlockState.generated.h"

USTRUCT(BlueprintType)
struct FBlockState 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block state")
	int64 blockID;

	FBlockState() { blockID = 0; }

	FBlockState(int64 ID) { blockID = ID; }
	FORCEINLINE
	bool operator==(const FBlockState& other) const 
	{
		return blockID == other.blockID;
	};
	FORCEINLINE
	bool IsAirBlock() const
	{
		return blockID == 0;
	}
	static FBlockState& AirBlock()
	{
		static FBlockState air{};
		return air;
	}

};


FORCEINLINE uint32 GetTypeHash(const FBlockState& block)
{
	uint32 Hash = FCrc::MemCrc32(&block, sizeof(block));
	return Hash;
}