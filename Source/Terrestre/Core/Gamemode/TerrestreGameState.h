// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TerrestreGameState.generated.h"

class AChunkManager;
class ARegionManager;
/**
 * 
 */
UCLASS()
class TERRESTRE_API ATerrestreGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	ATerrestreGameState();
	void ReceivePostLoadMapWithWorld(UWorld* InLoadedWorld);

	AChunkManager* GetChunkManager();

protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	TObjectPtr<AChunkManager> ChunkManager;

	UPROPERTY()
	TObjectPtr<ARegionManager> RegionManager;
};
