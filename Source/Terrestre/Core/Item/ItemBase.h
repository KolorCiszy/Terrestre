// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Terrestre/Core/Subsystems/ItemDataSubsystem/ItemData.h"
#include "Terrestre/Core/Subsystems/ItemDataSubsystem/ItemDataSubsystem.h"
#include "ItemBase.generated.h"

/**
 * Base class for stored or dropped items
 */
UCLASS(BlueprintType)
class TERRESTRE_API UItemBase : public UObject
{
	GENERATED_BODY()
public:
	UItemBase()
	{
		ItemID = 0;
		ItemCount = 1;
	};

	UItemBase(int32 InItemID, int32 InItemCount)
	{
		ItemID = InItemID;
		ItemCount = InItemCount;
	}
	virtual ~UItemBase() {};

	UFUNCTION(BlueprintCallable)
	void SetItemID(int32 itemID)
	{
		ItemID = itemID;
	}
	UFUNCTION(BlueprintPure)
	int32 GetItemID() const
	{
		return ItemID;
	}
	UFUNCTION(BlueprintCallable)
	void SetItemCount(int32 itemCount);
	
	UFUNCTION(BlueprintPure)
	int32 GetItemCount() const
	{
		return ItemCount;
	}
	UFUNCTION(BlueprintCallable)
	int32 HowManyCanStackWith(const UItemBase* itemToStack) const;
	
protected:
	UPROPERTY(BlueprintReadWrite)
	int32 ItemID;
	UPROPERTY(BlueprintReadWrite)
	int32 ItemCount;
};
