#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EChunkGenStage : uint8
{
	Empty,
	TerrainShape,
	SurfaceLayer,	
	Vegatation,
	Full,
	Count
	
};
 

FORCEINLINE EChunkGenStage operator++(EChunkGenStage GenStage)
{
	uint8 byte = static_cast<uint8>(GenStage);
	byte++;
	return EChunkGenStage(byte);
}

