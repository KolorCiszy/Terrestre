// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Terrestre/Core/Interfaces/InventoryComponent_I.h"
#include "Terrestre/Core/Item/ItemBase.h"
#include "InventoryComponent.generated.h"

class UItemBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TERRESTRE_API UInventoryComponent : public UActorComponent, public IInventoryComponent_I
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetSlotCount(int32 slotCount)
	{
		InventorySlotCount = slotCount;
		StoredItems.SetNumZeroed(InventorySlotCount, false);
	}
	UFUNCTION(BlueprintPure)
	int32 GetSlotCount() const
	{
		return InventorySlotCount;
	}
	/* Finds first empty slot index in inventory, -1 if inventory has no empty slots left*/
	UFUNCTION(BlueprintPure)
	int32 FindEmptyIndex() const;
	UFUNCTION(BlueprintCallable)
	UItemBase* GetStoredItemAtIndex(int32 slotIndex);
	UFUNCTION(BlueprintCallable)
	UItemBase* FindItemInStorage(const UItemBase* itemToFind);
	/* Add item to storage, stacking it when possible, default index (-1) will place the item in the first empty slot found,
	 * returns the amount of item that has been added to this storage */
	UFUNCTION(BlueprintCallable)
	int32 AddItemToStorage(UItemBase* item, int32 slotIndex = -1);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	int32 InventorySlotCount;
	
	UFUNCTION(BlueprintPure, Category = "Inventory Component")
	UItemBase* GetHeldItem_BP()
	{
		auto Item = GetHeldItem();
		return Item.Get();
	}

	TWeakObjectPtr<UItemBase> GetHeldItem()
	{
		return StoredItems[HeldItemIndex];
	}
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	void SetHeldItem(int32 indexInInventory)
	{
		if (StoredItems.IsValidIndex(indexInInventory))
		{
			HeldItemIndex = indexInInventory;
		}
	}
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	TArray<UItemBase*>& GetStoredItems()
	{
		return StoredItems;
	}
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void PostInitProperties() override;

private:
	UPROPERTY()
	TArray<UItemBase*> StoredItems;
	
	void InvalidateGarbageItems();

	int32 HeldItemIndex;
		
};
