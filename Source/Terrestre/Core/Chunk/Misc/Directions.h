#pragma once


#include "CoreMinimal.h"
UENUM(BlueprintType)
enum class EDirections
{
	Forward,Backward,Right,Left,Up,Down

};

ENUM_RANGE_BY_FIRST_AND_LAST(EDirections, EDirections::Forward, EDirections::Down);



