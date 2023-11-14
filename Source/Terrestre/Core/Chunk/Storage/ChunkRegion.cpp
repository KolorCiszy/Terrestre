#include "ChunkRegion.h"
FChunkProtoRegion::FChunkProtoRegion(FChunkRegion& packedData)
{
	ProtoChunkData.Reserve(FChunkRegion::RegionVolume);
	for (auto& chunk : packedData.ChunkData)
	{
		FProtoChunkData unpackedData = chunk.Value;
		ProtoChunkData.Add(chunk.Key, MoveTemp(unpackedData));
	}
}
FChunkProtoRegion& FChunkProtoRegion::operator=(FChunkRegion& packedData)
{
	ProtoChunkData.Reserve(FChunkRegion::RegionVolume);
	for (auto& chunk : packedData.ChunkData)
	{
		FProtoChunkData unpackedData = chunk.Value;
		ProtoChunkData.Add(chunk.Key, MoveTemp(unpackedData));
	}
	return *this;
}
