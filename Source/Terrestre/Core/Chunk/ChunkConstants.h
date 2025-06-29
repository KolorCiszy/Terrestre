#pragma once

#include "CoreMinimal.h"

struct FChunkConstants
{
	//* Size in regular blocks
	static inline constexpr int32 Size = 16;
	static inline constexpr int32 SizeSquared = Size * Size;
	static inline constexpr int32 Volume = SizeSquared * Size;

	//* Size of regular block in Unreal units
	static inline FVector VoxelSize{ 100.0, 100.0, 100.0 };
	
	static inline float VoxelSizeOneAxis{ static_cast<float>(VoxelSize.X) };
	//* Size of regular block in Unreal units casted to INT
	static inline FIntVector VoxelIntSize{ VoxelSize };
	//* Chunk size scaled by Unreal units 
	static inline FVector SizeScaled = VoxelSize * Size;


	/* Amount of chunks in one region on every axis */
	static constexpr int32 RegionSize = 8;
	/* Amount of chunks in one region on every axis squared */
	static constexpr int32 RegionSizeSquared = RegionSize * RegionSize;
	/* Amount of block in one region on one axis (length of an edge of region) */
	static constexpr int32 RegionSizeInBlocks = RegionSize * Size;
	/* Amount of block in one region on one axis (length of an edge of region) squared */
	static constexpr int32 RegionSizeInBlocksSquared = RegionSizeInBlocks * RegionSizeInBlocks;

	static constexpr int32 RegionVolumeInBlocks = RegionSizeInBlocks * RegionSizeInBlocksSquared;
	/* Amount of chunks in one region (volume of region in chunks) */
	static constexpr int32 RegionVolume = RegionSize * RegionSize * RegionSize;
	/* The size of chunk region on every axis, scaled to unreal units */
	static inline FIntVector RegionSizeScaled{ RegionSize * SizeScaled };
};