// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chunk.h"
#include "Storage/ChunkRegion.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChunkUtilityLib.generated.h"


class AChunkManager;


inline FIntVector operator/(const FIntVector& a, const FIntVector& b)
{
	FIntVector result;
	result.X = a.X / b.X;
	result.Y = a.Y / b.Y;
	result.Z = a.Z / b.Z;
	return result;
}
inline FIntVector& operator/=(FIntVector& a, const FIntVector& div)
{
	a = a / div;
	return a;
}
inline bool operator<(const FVector& a, const FVector& b)
{
	return a.SquaredLength() < b.SquaredLength();
}


/**
 * 
 */
UCLASS()
class TERRESTRE_API UChunkUtilityLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	friend class AChunkManager;
public:
	UFUNCTION(BlueprintPure, Category = "Chunk Manager")
	static AChunkManager* GetChunkManager()
	{
		return ChunkManager;
	}
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FVector WorldLocationToChunkLocation(FVector inWorldLocation)
	{
		FVector result{};
		result.X = FMath::TruncToInt32(inWorldLocation.X) / FMath::TruncToInt32(AChunk::SizeScaled.X);
		result.Y = FMath::TruncToInt32(inWorldLocation.Y) / FMath::TruncToInt32(AChunk::SizeScaled.Y);
		result.Z = FMath::TruncToInt32(inWorldLocation.Z) / FMath::TruncToInt32(AChunk::SizeScaled.Z);
		if (inWorldLocation.X < 0)
			result.X--;
		if (inWorldLocation.Y < 0)
			result.Y--;
		if (inWorldLocation.Z < 0)
			result.Z--;
		result *= AChunk::SizeScaled;
		return result;
	}
	
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FIntVector WorldLocationToLocalBlockPos(FVector inWorldLocation)
	{
		FIntVector localBlockPos{};
		FVector chunkLocation{ WorldLocationToChunkLocation(inWorldLocation) };
		localBlockPos.X = FMath::Floor((inWorldLocation.X - chunkLocation.X) / AChunk::VoxelSize.X);
		localBlockPos.Y = FMath::Floor((inWorldLocation.Y - chunkLocation.Y) / AChunk::VoxelSize.Y);
		localBlockPos.Z = FMath::Floor((inWorldLocation.Z - chunkLocation.Z) / AChunk::VoxelSize.Z);
		if(localBlockPos.X < 0)
		{
			localBlockPos.X += AChunk::Size;
		}
		if (localBlockPos.Y < 0)
		{
			localBlockPos.Y += AChunk::Size;
		}
		if (localBlockPos.Z < 0)
		{
			localBlockPos.Z += AChunk::Size;
		}
		localBlockPos %= AChunk::Size;
		
		return localBlockPos;
	}
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static int32 LocalBlockPosToIndex(const FIntVector localPos)
	{
		return localPos.X + localPos.Y * AChunk::Size + localPos.Z * AChunk::SizeSquared;
	}

	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static bool IsValidLocalIndex(const int32 index)
	{
		return index < AChunk::Volume && index >= 0;
	}

	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FIntVector IndexToLocalBlockPos(int32 index)
	{
		FIntVector result{};
		result.Z = index / AChunk::SizeSquared;
		index -= result.Z * AChunk::SizeSquared;
		result.Y = index / AChunk::Size;
		result.X = index % AChunk::Size;
		return result;
	}

	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static bool IsValidLocalPosition(FIntVector localPos)
	{
		return (localPos.X >= 0 && localPos.X < AChunk::Size &&
				localPos.Y >= 0 && localPos.Y < AChunk::Size &&
				localPos.Z >= 0 && localPos.Z < AChunk::Size);
	}

	/* Eg. position X = 122, Y = 55, Z = 313 becomes X = 200, Y = 0, Z = 400 */
	/* X = -50, Y = -30, Z = -301 becomes X = -100, Y = -100, Z = -400 */
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FVector SnapWorldLocationToWorldBlockLocation(FVector inWorldLocation)
	{

		if(inWorldLocation.X < 0)
		{
			inWorldLocation.X = FMath::TruncToInt64(inWorldLocation.X) / AChunk::VoxelIntSize.X;
			inWorldLocation.X--;
		}
		else
		{
			inWorldLocation.X = FMath::TruncToInt64(inWorldLocation.X) / AChunk::VoxelIntSize.X;
		}
		if(inWorldLocation.Y < 0)
		{
			inWorldLocation.Y = FMath::TruncToInt64(inWorldLocation.Y) / AChunk::VoxelIntSize.Y;
			inWorldLocation.Y--;
		}
		else
		{
			inWorldLocation.Y = FMath::TruncToInt64(inWorldLocation.Y) / AChunk::VoxelIntSize.Y;
		}
		if (inWorldLocation.Z < 0)
		{
			inWorldLocation.Z = FMath::TruncToInt64(inWorldLocation.Z) / AChunk::VoxelIntSize.Z;
			inWorldLocation.Z--;
		}
		else
		{
			inWorldLocation.Z = FMath::TruncToInt64(inWorldLocation.Z) / AChunk::VoxelIntSize.Z;
		}
		inWorldLocation *= AChunk::VoxelSize;		//* quite ugly
		return inWorldLocation;
	}

	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static inline double SetFloatPrecision(double inFloat, int32 decimalPlaces)
	{
		int64 decimalPointShift = FMath::Pow(10.0, decimalPlaces);
		inFloat *= decimalPointShift;
		double temp = FMath::TruncToDouble(inFloat);
		return temp / decimalPointShift;
	}


	/* Converets chunk world location to relative location eg. 
	* X = 1600, Y = 0, Z = - 1600 becomes
	  X = 1, Y = 0, Z = -1 */
	static FIntVector ChunkWorldLocationToRelativeLocation(FVector worldLocation)
	{
		FIntVector result{worldLocation};
		result.X /= FMath::FloorToInt32(AChunk::SizeScaled.X);
		result.Y /= FMath::FloorToInt32(AChunk::SizeScaled.Y);
		result.Z /= FMath::FloorToInt32(AChunk::SizeScaled.Z);
		return result;
	}

	/* Get the region ID which this chunk belongs to, has to be the exact chunk location */
	static FIntVector GetRegionID(FVector chunkWorldLocation)
	{
		FIntVector relativeLocation = ChunkWorldLocationToRelativeLocation(chunkWorldLocation);
		FIntVector RegID{ relativeLocation };
		uint8 bitShift = FMath::Log2(float(FChunkRegion::RegionSize));
		RegID.X >>= bitShift;
		RegID.Y >>= bitShift;
		RegID.Z >>= bitShift;
		return RegID;
	}
	

private:
	static inline TObjectPtr<AChunkManager> ChunkManager;
};
