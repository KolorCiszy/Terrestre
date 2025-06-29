// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemData.generated.h"

class AEntityItem;

/**
 * 
 */
USTRUCT(BlueprintType)
struct TERRESTRE_API FItemData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item data")
	FName DisplayName = TEXT("NONE");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item data")
	TSoftObjectPtr<UTexture2D> DisplayTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item data")
	int32 MaxStackSize = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item data")
	TSoftObjectPtr<UStaticMesh> StaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item data")
	TSoftClassPtr<AEntityItem> SpawnEntityClass;
	
	
};



