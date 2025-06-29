// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionResult.h"
#include "InteractableActor.generated.h"


class ABaseCharacter;


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
	FInteractionResult OnLeftMouseButton(ABaseCharacter* clickedBy, const FHitResult& traceResult);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Actor")
	FInteractionResult OnRightMouseButton(ABaseCharacter* clickedBy, const FHitResult& traceResult);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Actor")
	FInteractionResult OnVisibleByCharacter(ABaseCharacter* visibleBy, const FHitResult& traceResult);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Actor")
	FInteractionResult OnInteractedWith(ABaseCharacter* interactedBy, const FHitResult& traceResult);
};
