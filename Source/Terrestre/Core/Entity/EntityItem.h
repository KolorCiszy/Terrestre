// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EntityBase.h"
#include "Terrestre/Core/Item/ItemBase.h"
#include "Terrestre/Core/Interfaces/InventoryComponent_I.h"
#include "EntityItem.generated.h"


/* This class is a base for all dynamic objects spawned in the world */
UCLASS()
class TERRESTRE_API AEntityItem : public AEntityBase, public IInventoryComponent_I
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEntityItem();

	virtual void SetSpawnParams(int32 ItemCount, int32 ItemID);
	
	virtual ~AEntityItem();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	
	UFUNCTION(BlueprintCallable)
	void UpdateBoundItem(UItemBase* _boundItem);

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UItemBase> BoundItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 ItemCountInThisStack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ItemMesh;


	

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
