#pragma once

#include "CoreMinimal.h"
#include "FluidState.generated.h"

USTRUCT(BlueprintType)
struct FFluidState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid state")
	uint8 fluidID; /* ID 0 represents an "empty" fluid, ID UINT8MAX represents an index, to which fluid can't move to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid state")
	uint8 fillLevel; /* percentage; 0 - 0% fill, 100 - 100% fill*/

	FFluidState() { fluidID = 0; fillLevel = 0; }
	
	FFluidState(uint8 inFluidID, uint8 inFillLevel) { fluidID = inFluidID; fillLevel = inFillLevel; }
	FORCEINLINE
	bool operator==(const FFluidState& other) const
	{
		return fluidID == other.fluidID;
	};
	
	bool IsWater()
	{
		return fluidID == 1;
	}
	static FFluidState& Empty()
	{
		static FFluidState empty{};
		return empty;
	}
};
FORCEINLINE FArchive& operator<<(FArchive& ar, FFluidState fs)
{
	ar << fs.fluidID;
	ar << fs.fillLevel;
	return ar;
}