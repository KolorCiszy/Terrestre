// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase.h"

void UItemBase::SetItemCount(int32 itemCount)
{
	ItemCount = itemCount;
	check(ItemCount >= 0)
		if (ItemCount == 0)
		{
			MarkAsGarbage();
			ConditionalBeginDestroy();
		}
}
int32 UItemBase::HowManyCanStackWith(const UItemBase* itemToStack) const
{
	const FItemData* ItemData = GEngine->GetEngineSubsystem<UItemDataSubsystem>()->GetItemData(itemToStack->ItemID);
	int32 maxStackSize = ItemData->MaxStackSize;

	return FMath::Clamp(maxStackSize - ItemCount, 0, FMath::Min(maxStackSize, itemToStack->GetItemCount()));
}
