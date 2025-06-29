// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "Terrestre/Core/Gamemode/PlayerGameMode.h"
#include "TerrestreCheatManager.generated.h"

/**
 * 
 */
UCLASS(Within=TerrestrePlayerController)
class TERRESTRE_API UTerrestreCheatManager : public UCheatManager
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Exec, Category = "Cheat Manager")
	void gm(EPlayerGameMode newGameMode);

	
};
