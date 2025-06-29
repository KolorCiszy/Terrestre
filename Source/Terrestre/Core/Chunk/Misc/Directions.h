#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EDirections : uint8
{
	Forward,
	Backward,
	Right,
	Left,
	Up,
	Down,
	NONE,
	COUNT UMETA(Hidden)
};
ENUM_RANGE_BY_FIRST_AND_LAST(EDirections, EDirections::Forward, EDirections::Down);

static inline EDirections GetDirectionFromNormalVector(FVector normal)
{
	if (normal == FVector::ForwardVector)
	{
		return EDirections::Forward;
	}
	else if (normal == FVector::BackwardVector)
	{
		return EDirections::Backward;
	}
	else if (normal == FVector::RightVector)
	{
		return EDirections::Right;
	}
	else if (normal == FVector::LeftVector)
	{
		return EDirections::Left;
	}
	else if (normal == FVector::UpVector)
	{
		return EDirections::Up;
	}
	else if (normal == FVector::DownVector)
	{
		return EDirections::Down;
	}
	return EDirections::NONE;
};






