#pragma once

#include "CoreMinimal.h"
#include "Storage/ChunkData.h"

class AChunk;

class IChunkProviderInterface
{
public:
	virtual TFuture<FChunkData> ProvideChunkData(AChunk* Chunk) { return TFuture<FChunkData>(); };
};