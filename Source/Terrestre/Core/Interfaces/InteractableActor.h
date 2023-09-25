// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableActor.generated.h"


class ABaseCharacter;

/*
USTRUCT(BlueprintType)
struct FInteractionResult
{
	GENERATED_BODY()

	UPROPERTY()
	FHitResult traceResult;

	UPROPERTY()
	bool bInteractionSuccess;
};
*/
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TERRESTRE_API IInteractableActor
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Actor")
	bool OnLeftMouseButton(ABaseCharacter* clickedBy, const FHitResult& traceResult, int64 heldItemID);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Actor")
	bool OnRightMouseButton(ABaseCharacter* clickedBy, const FHitResult& traceResult, int64 heldItemID);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Actor")
	FName OnVisibleByCharacter(ABaseCharacter* visibleBy, const FHitResult& traceResult);
};
