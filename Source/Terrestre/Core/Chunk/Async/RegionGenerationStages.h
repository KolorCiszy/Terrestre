#pragma once

#include "CoreMinimal.h"

enum class ERegionGenerationStages : uint8
{
	None,
	TerrainShape,
	SurfaceLayer,
	Vegatation,
	Full,
	Count
};
