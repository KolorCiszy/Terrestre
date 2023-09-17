// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TerrestreGameModeBase.generated.h"

class APlayerCharacter;
class ATerrestrePlayerState;
/**
 * 
 */
UCLASS()
class TERRESTRE_API ATerrestreGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATerrestreGameModeBase();
	APlayerCharacter* SpawnPlayerCharacter(APlayerState* owner);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<APlayerCharacter> PlayerCharacterClass;
	
	
	// List of PlayerControllers
	UPROPERTY()
	TArray<APlayerController*> PlayerControllerList;
protected:
	virtual void BeginPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	// Overriding Logout funciton
	virtual void Logout(AController* Exiting) override;
};
