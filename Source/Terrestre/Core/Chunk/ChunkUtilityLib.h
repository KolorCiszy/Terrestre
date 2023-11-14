// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chunk.h"
#include "Misc/Directions.h"
#include "Storage/ChunkRegion.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Terrestre/Core/Misc/ArrayHelpers.h"
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
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static UTerrainShaper* GetTerrainShaper()
	{
		return TerrainShaper;
	}
	UFUNCTION(BlueprintPure, Category = "Terrain Shaper")
	static UTerrainSurfaceDecorator* GetTerrainSurfaceDecorator()
	{
		return TerrainSurfaceDecorator;
	}
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FVector WorldLocationToChunkLocation_BP(FVector inWorldLocation)
	{
		return WorldLocationToChunkLocation(inWorldLocation);
	}

	/* Converts world location (in ue units) to world block position */
	static FORCEINLINE FIntVector WorldLocationToBlockPos(FVector inWorldLocation)
	{
		FIntVector output{};
		output.X = FMath::FloorToInt32(inWorldLocation.X / AChunk::VoxelSize.X);
		output.Y = FMath::FloorToInt32(inWorldLocation.Y / AChunk::VoxelSize.Y);
		output.Z = FMath::FloorToInt32(inWorldLocation.Z / AChunk::VoxelSize.Z);
		return output;
	}

	static FVector WorldLocationToChunkLocation(FVector inWorldLocation)
	{
		FVector result{};
		result.X = FMath::Floor(inWorldLocation.X / AChunk::SizeScaled.X);
		result.Y = FMath::Floor(inWorldLocation.Y / AChunk::SizeScaled.Y);
		result.Z = FMath::Floor(inWorldLocation.Z / AChunk::SizeScaled.Z);
		result *= AChunk::SizeScaled;
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
	
	static constexpr FVector GetNeighbourChunkLocation(FVector fromLocation, EDirections direction)
	{
		
		switch (direction)
		{
		case EDirections::Forward: fromLocation.X += AChunk::SizeScaled.X;
			break;
		case EDirections::Backward:fromLocation.X -= AChunk::SizeScaled.X;
			break;
		case EDirections::Up: fromLocation.Z += AChunk::SizeScaled.Z;
			break;
		case EDirections::Down:fromLocation.Z -= AChunk::SizeScaled.Z;
			break;
		case EDirections::Left: fromLocation.Y -= AChunk::SizeScaled.Y;
			break;
		case EDirections::Right: fromLocation.Y += AChunk::SizeScaled.Y;
			break;
		default: return fromLocation;
		}
		return fromLocation;
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
	/* Converts world block pos to chunk world location it belongs to */
	static FVector BlockPosToChunkWorldLocation(FIntVector blockPos)
	{
		blockPos.X = FMath::Floor(double(blockPos.X) / double(AChunk::Size));
		blockPos.Y = FMath::Floor(double(blockPos.Y) / double(AChunk::Size));
		blockPos.Z = FMath::Floor(double(blockPos.Z) / double(AChunk::Size));
		return FVector(blockPos) *= AChunk::SizeScaled;
	}
	template<class T, class C>
	static inline C Modulo(T a, C b)
	{
		static_assert(TIsIntegral<T>::Value && TIsIntegral<C>::Value);
		return a - (b * FMath::FloorToInt64(double(a) / double(b)));
	}
	/* Converts world block position to chunk's local block position */
	static FIntVector BlockPosToLocalBlockPos(FIntVector blockPos)
	{
		blockPos.X = Modulo(blockPos.X, AChunk::Size);
		blockPos.Y = Modulo(blockPos.Y, AChunk::Size);
		blockPos.Z = Modulo(blockPos.Z, AChunk::Size);
		return blockPos;
	}
	/* Converets chunk world location to relative location eg. 
	* X = 1600, Y = 0, Z = -1600 becomes
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
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
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
	/* Get region origin in block coordinates */
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static FIntVector GetRegionOrigin(FIntVector regionID)
	{
		int32 OriginX = regionID.X * FChunkRegion::RegionSizeInBlocks;
		int32 OriginY = regionID.Y * FChunkRegion::RegionSizeInBlocks;
		int32 OriginZ = regionID.Z * FChunkRegion::RegionSizeInBlocks;
		/*
		if(regionID.X < 0)
		{
			OriginX--;
		}
		if (regionID.Y < 0)
		{
			OriginY--;
		}
		if (regionID.Z < 0)
		{
			OriginZ--;
		}
		*/
		return FIntVector{ OriginX,OriginY,OriginZ };
	}
	UFUNCTION(BlueprintPure, Category = "Chunk Utilities")
	static inline FVector GetRegionOriginWorldSpace(FIntVector regionID)
	{
		return FVector{ regionID * FChunkRegion::RegionSizeScaled };
	}

private:
	static inline AChunkManager* ChunkManager;
	static inline UTerrainShaper* TerrainShaper;
	static inline UTerrainSurfaceDecorator* TerrainSurfaceDecorator;
};
