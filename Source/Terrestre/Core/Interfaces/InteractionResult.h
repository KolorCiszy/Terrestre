#pragma once


#include "CoreMinimal.h"
#include "InteractionResult.generated.h"

USTRUCT(BlueprintType)
struct FInteractionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	AActor* InteractedWith;
	UPROPERTY(BlueprintReadWrite)
	bool bSuccess;
	UPROPERTY(BlueprintReadWrite)
	bool bShouldRefreshInventoryUI;
};