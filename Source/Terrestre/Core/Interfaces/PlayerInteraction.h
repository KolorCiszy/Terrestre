// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInteraction.generated.h"

class UItemBase;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInteraction : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implement this interface to perform various player related actions
 */
class TERRESTRE_API IPlayerInteraction
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Player Interaction")
	UItemBase* GetHeldItem();
};
