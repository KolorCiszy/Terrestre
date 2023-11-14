#pragma once

#include "CoreMinimal.h"

namespace ArrayHelpers
{
	static constexpr FORCEINLINE FIntVector2 IndexToPos2D(size_t index, size_t arraySize)
	{
		FIntVector2 result{};
		result.Y = index / arraySize;
		result.X = index % arraySize;
		return result;
	}
	static constexpr FORCEINLINE size_t Pos2DToIndex(FIntVector2 pos, size_t arraySize)
	{
		return pos.X + pos.Y * arraySize;
	}
	static constexpr FORCEINLINE FIntVector IndexToPos3D(size_t index, size_t arraySize)
	{
		FIntVector result{};
		result.Z = index / arraySize;
		index -= result.Z * arraySize;
		result.Y = index / arraySize;
		result.X = index % arraySize;
		return result;
	}

}