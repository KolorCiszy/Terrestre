﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshActor.h"
#include "RealtimeMeshStressTestActor.generated.h"

UCLASS()
class REALTIMEMESHTESTS_API ARealtimeMeshStressTestActor : public ARealtimeMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARealtimeMeshStressTestActor();

	virtual void OnGenerateMesh_Implementation() override;

	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
};

