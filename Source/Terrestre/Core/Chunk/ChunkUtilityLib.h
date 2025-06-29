// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkConstants.h"
#include "Misc/Directions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Terrestre/Core/Misc/ArrayHelpers.h"
#include "ChunkUtilityLib.generated.h"


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

public:
	
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FVector WorldLocationToChunkLocation_BP(FVector inWorldLocation)
	{
		return WorldLocationToChunkLocation(inWorldLocation);
	}

	/* Converts world location (in ue units) to world block position */
	static FORCEINLINE FIntVector WorldLocationToBlockPos(FVector inWorldLocation)
	{
		FIntVector output{};
		output.X = FMath::FloorToInt32(inWorldLocation.X / FChunkConstants::VoxelSize.X);
		output.Y = FMath::FloorToInt32(inWorldLocation.Y / FChunkConstants::VoxelSize.Y);
		output.Z = FMath::FloorToInt32(inWorldLocation.Z / FChunkConstants::VoxelSize.Z);
		return output;
	}

	static FVector WorldLocationToChunkLocation(FVector inWorldLocation)
	{
		FVector result{};
		result.X = FMath::Floor(inWorldLocation.X / FChunkConstants::SizeScaled.X);
		result.Y = FMath::Floor(inWorldLocation.Y / FChunkConstants::SizeScaled.Y);
		result.Z = FMath::Floor(inWorldLocation.Z / FChunkConstants::SizeScaled.Z);
		result *= FChunkConstants::SizeScaled;
		return result;
	}
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FVector LocalPositionToWorldLocation(const FIntVector& localPos, const FVector& chunkWorldLocation)
	{
		FVector result{};
		result.X = localPos.X * FChunkConstants::VoxelSize.X + chunkWorldLocation.X;
		result.Y = localPos.Y * FChunkConstants::VoxelSize.Y + chunkWorldLocation.Y;
		result.Z = localPos.Z * FChunkConstants::VoxelSize.Z + chunkWorldLocation.Z;
		return result;
	}
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FIntVector WorldLocationToLocalBlockPos(FVector inWorldLocation)
	{
		FIntVector worldBlockPos{ WorldLocationToBlockPos(inWorldLocation) };
		return BlockPosToLocalBlockPos(worldBlockPos);
	}
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static int32 LocalBlockPosToIndex_BP(const FIntVector localPos)
	{
		return LocalBlockPosToIndex(localPos);
	}

	static constexpr FORCEINLINE int32 LocalBlockPosToIndex(const FIntVector localPos)
	{
		return localPos.X + localPos.Y * FChunkConstants::Size + localPos.Z * FChunkConstants::SizeSquared;
	}

	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static bool IsValidLocalIndex(const int32 index)
	{
		return index < FChunkConstants::Volume && index >= 0;
	}

	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FIntVector IndexToLocalBlockPos(int32 index)
	{
		FIntVector result{};
		result.Z = index / FChunkConstants::SizeSquared;
		index -= result.Z * FChunkConstants::SizeSquared;
		result.Y = index / FChunkConstants::Size;
		result.X = index % FChunkConstants::Size;
		return result;
	}
	
	static constexpr FVector GetNeighbourChunkLocation(FVector fromLocation, EDirections direction)
	{
		
		switch (direction)
		{
		case EDirections::Forward: fromLocation.X += FChunkConstants::SizeScaled.X;
			break;
		case EDirections::Backward:fromLocation.X -= FChunkConstants::SizeScaled.X;
			break;
		case EDirections::Up: fromLocation.Z += FChunkConstants::SizeScaled.Z;
			break;
		case EDirections::Down:fromLocation.Z -= FChunkConstants::SizeScaled.Z;
			break;
		case EDirections::Left: fromLocation.Y -= FChunkConstants::SizeScaled.Y;
			break;
		case EDirections::Right: fromLocation.Y += FChunkConstants::SizeScaled.Y;
			break;
		default: return fromLocation;
		}
		return fromLocation;
	}
	

	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static bool IsValidLocalPosition(FIntVector localPos)
	{
		return (localPos.X >= 0 && localPos.X < FChunkConstants::Size &&
				localPos.Y >= 0 && localPos.Y < FChunkConstants::Size &&
				localPos.Z >= 0 && localPos.Z < FChunkConstants::Size);
	}

	/* Eg. position X = 122, Y = 55, Z = 313 becomes X = 200, Y = 0, Z = 400 */
	/* X = -50, Y = -30, Z = -301 becomes X = -100, Y = -100, Z = -400 */
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FVector SnapWorldLocationToWorldBlockLocation(FVector inWorldLocation)
	{

		if(inWorldLocation.X < 0)
		{
			inWorldLocation.X = FMath::TruncToInt64(inWorldLocation.X) / FChunkConstants::VoxelIntSize.X;
			inWorldLocation.X--;
		}
		else
		{
			inWorldLocation.X = FMath::TruncToInt64(inWorldLocation.X) / FChunkConstants::VoxelIntSize.X;
		}
		if(inWorldLocation.Y < 0)
		{
			inWorldLocation.Y = FMath::TruncToInt64(inWorldLocation.Y) / FChunkConstants::VoxelIntSize.Y;
			inWorldLocation.Y--;
		}
		else
		{
			inWorldLocation.Y = FMath::TruncToInt64(inWorldLocation.Y) / FChunkConstants::VoxelIntSize.Y;
		}
		if (inWorldLocation.Z < 0)
		{
			inWorldLocation.Z = FMath::TruncToInt64(inWorldLocation.Z) / FChunkConstants::VoxelIntSize.Z;
			inWorldLocation.Z--;
		}
		else
		{
			inWorldLocation.Z = FMath::TruncToInt64(inWorldLocation.Z) / FChunkConstants::VoxelIntSize.Z;
		}
		inWorldLocation *= FChunkConstants::VoxelSize;		//* quite ugly
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
	/* Converts world block pos to chunk world location it belongs to */
	static FVector BlockPosToChunkWorldLocation(FIntVector blockPos)
	{
		blockPos.X = FMath::Floor(double(blockPos.X) / double(FChunkConstants::Size));
		blockPos.Y = FMath::Floor(double(blockPos.Y) / double(FChunkConstants::Size));
		blockPos.Z = FMath::Floor(double(blockPos.Z) / double(FChunkConstants::Size));
		return FVector(blockPos) *= FChunkConstants::SizeScaled;
	}
	template<class T, class C> /* a % b */
	static inline C Modulo(T a, C b)
	{
		static_assert(TIsIntegral<T>::Value && TIsIntegral<C>::Value);
		return a - (b * FMath::FloorToInt64(double(a) / double(b)));
	}
	static inline FIntVector VecModulo(FIntVector a, int32 b)
	{
		FIntVector result{};
		result.X = Modulo(a.X, b);
		result.Y = Modulo(a.Y, b);
		result.Z = Modulo(a.Z, b);
		return result;
	}
	/* Converts world block position to chunk's local block position */
	static FIntVector BlockPosToLocalBlockPos(FIntVector blockPos)
	{
		blockPos.X = Modulo(blockPos.X, FChunkConstants::Size);
		blockPos.Y = Modulo(blockPos.Y, FChunkConstants::Size);
		blockPos.Z = Modulo(blockPos.Z, FChunkConstants::Size);
		return blockPos;
	}
	/* Converets chunk world location to relative location eg. 
	* X = 1600, Y = 0, Z = -1600 becomes
	  X = 1, Y = 0, Z = -1 */
	static FIntVector ChunkWorldLocationToRelativeLocation(FVector worldLocation)
	{
		FIntVector result{worldLocation};
		result.X /= FMath::FloorToInt32(FChunkConstants::SizeScaled.X);
		result.Y /= FMath::FloorToInt32(FChunkConstants::SizeScaled.Y);
		result.Z /= FMath::FloorToInt32(FChunkConstants::SizeScaled.Z);
		return result;
	}

	/* Get the region ID which this chunk belongs to, has to be the exact chunk location */
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FIntVector GetRegionID(FVector chunkWorldLocation)
	{
		FIntVector relativeLocation = ChunkWorldLocationToRelativeLocation(chunkWorldLocation);
		FIntVector RegID{ relativeLocation };
		uint8 bitShift = FMath::Log2(float(FChunkConstants::RegionSize));
		RegID.X >>= bitShift;
		RegID.Y >>= bitShift;
		RegID.Z >>= bitShift;
		return RegID;
	}
	/* Get region origin in block coordinates */
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FIntVector GetRegionOrigin(FIntVector regionID)
	{
		int32 OriginX = regionID.X * FChunkConstants::RegionSizeInBlocks;
		int32 OriginY = regionID.Y * FChunkConstants::RegionSizeInBlocks;
		int32 OriginZ = regionID.Z * FChunkConstants::RegionSizeInBlocks;
		
		return FIntVector{ OriginX,OriginY,OriginZ };
	}
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static inline FVector GetRegionOriginWorldSpace(FIntVector regionID)
	{
		return FVector{ regionID * FChunkConstants::RegionSizeScaled };
	}

};
