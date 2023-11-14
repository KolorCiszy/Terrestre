#pragma once

#include "CoreMinimal.h"
#include "BlockState.generated.h"

#define TERRESTRE_BLOCK_AIR 0
#define TERRESTRE_BLOCK_BASALT 1
#define TERRESTRE_BLOCK_GRASS 2
#define TERRESTRE_BLOCK_DIRT 3
#define TERRESTRE_BLOCK_SAND 4

USTRUCT(BlueprintType)
struct FBlockState 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block state")
	int32 blockID;

	FBlockState() { blockID = 0; }

	FBlockState(int32 ID) { blockID = ID; }
	FORCEINLINE
	bool operator==(const FBlockState& other) const 
	{
		return blockID == other.blockID;
	};
	FORCEINLINE
	bool operator!=(const FBlockState& other) const
	{
		return blockID != other.blockID;
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
	bool CanCharacterStandOn() const
	{
		return !IsAirBlock();
	}
};



FORCEINLINE uint32 GetTypeHash(const FBlockState& block)
{
	uint32 Hash = FCrc::MemCrc32(&block, sizeof(block));
	return Hash;
}