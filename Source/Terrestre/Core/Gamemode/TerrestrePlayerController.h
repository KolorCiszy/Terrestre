// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TerrestrePlayerController.generated.h"

/**
 * 
 */
UCLASS()	
class TERRESTRE_API ATerrestrePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetSelectedHotbarSlot(uint8 index);
	UFUNCTION(BlueprintPure)
	bool IsHotbarSlotSelected(uint8 index);
	UFUNCTION(BlueprintPure)
	uint8 GetSelectedHotbarSlot();

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateHotbarWidget();
	virtual void OnUpdateHotbarWidget_Implemenetation();

// Actor overrides
	//virtual void Tick(float DeltaTime) override;


private:
	UPROPERTY()
	uint8 SelectedHotbarSlot;

	
};
