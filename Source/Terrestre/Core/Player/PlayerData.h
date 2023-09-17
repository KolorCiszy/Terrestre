#pragma once

#include "CoreMinimal.h"


struct FPlayerData
{
	FTransform Transform;
	FString Name;
};

FArchive& operator<<(FArchive& ar, FPlayerData& data)
{
	ar << data.Name;
	ar << data.Transform;
	return ar;
}