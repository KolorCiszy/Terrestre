// Fill out your copyright notice in the Description page of Project Settings.

#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "ChunkManager.h"

#include "Kismet/GameplayStatics.h"


AChunkManager* UChunkUtilityLib::GetChunkManager()
{
	return ChunkManager;
}
FVector UChunkUtilityLib::WorldLocationToChunkLocation(FVector inWorldLocation)
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
/*
FIntVector UChunkUtilityLib::WorldLocationToLocalBlockPos(FVector inWorldLocation)
{

	FVector chunkWorldLocation = WorldLocationToChunkLocation(inWorldLocation);
	FIntVector result{ FVector{inWorldLocation - chunkWorldLocation}.GetAbs()};
	

	result.X = int32(abs(inWorldLocation.X - chunkWorldLocation.X)) / int32(AChunk::VoxelSize.X);
	result.Y = int32(abs(inWorldLocation.Y - chunkWorldLocation.Y)) / int32(AChunk::VoxelSize.Y);
	result.Z = int32(abs(inWorldLocation.Z - chunkWorldLocation.Z)) / int32(AChunk::VoxelSize.Z);
	/*
	result.X /= FMath::TruncToInt32(AChunk::VoxelSize.X);
	result.Y /= FMath::TruncToInt32(AChunk::VoxelSize.Y);
	result.Z /= FMath::TruncToInt32(AChunk::VoxelSize.Z);
	/*
	if (inWorldLocation.X < 0)
		result.X--;
	if (inWorldLocation.Y < 0)
		result.Y--;
	if (inWorldLocation.Z < 0)
		result.Z--;
		
	return result;
}
*/