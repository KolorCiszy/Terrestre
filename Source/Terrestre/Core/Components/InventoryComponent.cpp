// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Components/InventoryComponent.h"
#include "Terrestre/Core/Item/ItemBase.h"


// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
}
void UInventoryComponent::PostInitProperties()
{
	Super::PostInitProperties();
	
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	StoredItems.SetNumZeroed(InventorySlotCount, false);
}
UItemBase* UInventoryComponent::GetStoredItemAtIndex(int32 slotIndex)
{
	if (StoredItems.IsValidIndex(slotIndex))
	{
		return StoredItems[slotIndex];
	}
	return nullptr;
}
int32 UInventoryComponent::FindEmptyIndex() const
{
	int32 index = -1;
	for (auto& storageEntry : StoredItems)
	{
		index++;
		if (storageEntry == nullptr)
		{
			break;
		}	
	}
	return index;
}
UItemBase* UInventoryComponent::FindItemInStorage(const UItemBase* itemToFind)
{
	for (auto& storedItem : StoredItems)
	{
		if (storedItem)
		{
			if (storedItem->GetItemID() == itemToFind->GetItemID())
			{
				return storedItem;
			}
				
		}
		else
		{
			continue;
		}
	}
	return nullptr;
}
int32 UInventoryComponent::AddItemToStorage(UItemBase* item, int32 index)
{
	InvalidateGarbageItems();
	for (auto& storedItem : StoredItems)
	{
		if (storedItem)
		{
			if (storedItem->GetItemID() == item->GetItemID())
			{
				int32 howManyToStack = storedItem->HowManyCanStackWith(item);
				if (howManyToStack)
				{
					storedItem->SetItemCount(storedItem->GetItemCount() + howManyToStack);
					item->SetItemCount(item->GetItemCount() - howManyToStack);
					return howManyToStack;
				}
				else
				{
					continue;
				}
			}
		}
	}
// failed to stack the new item with existing ones, so add it as new stack
	if (index != -1)
	{
		if (StoredItems.IsValidIndex(index))
		{
			StoredItems[index] = item;
			return item->GetItemCount();
		}
		return 0;
	}
	else
	{
		int32 emptyIndex = FindEmptyIndex();
		if (emptyIndex != -1)
		{
			StoredItems[emptyIndex] = item;
			return item->GetItemCount();
		}
		return 0;
	}	
	
}
void UInventoryComponent::InvalidateGarbageItems()
{
	for (auto& storedItem : StoredItems)
	{
		if (storedItem)
		{
			if (!IsValid(storedItem))
			{
				storedItem = nullptr;
			}
		}
	}
}
// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

