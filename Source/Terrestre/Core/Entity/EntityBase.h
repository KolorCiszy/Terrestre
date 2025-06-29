// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Terrestre/Core/Interfaces/InteractableActor.h"
#include "EntityBase.generated.h"

/* This class is a base for all dynamic objects spawned in the world */
UCLASS(Abstract)
class TERRESTRE_API AEntityBase : public AActor, public IInteractableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEntityBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
