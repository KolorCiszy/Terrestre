#include "ChunkData.h"

FProtoChunkData::FProtoChunkData(const FChunkData& PackedData)
{
	PackedData.BlockPalette.BulkUnpack(BlockStates);
	FluidStates = PackedData.FluidStates;
}
FProtoChunkData& FProtoChunkData::operator=(const FChunkData& PackedData)
{
	PackedData.BlockPalette.BulkUnpack(BlockStates);
	FluidStates = PackedData.FluidStates;
	return *this;
}