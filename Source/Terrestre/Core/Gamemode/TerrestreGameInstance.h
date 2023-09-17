// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "TerrestreGameInstance.generated.h"





/**
 * 
 */
UCLASS()
class TERRESTRE_API UTerrestreGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Exec, Category = "Chunk manager commands")
	void SetChunkManagerTickEnabled(bool bEnabled);
	
	UFUNCTION(BlueprintCallable, Exec, Category = "Chunk manager commands")
	void SetRenderDistance(uint8 newDistance);

	
};
