#pragma once

#include "CoreMinimal.h"
#include "ItemEntityFactory.generated.h"
class AEntityItem;
UCLASS()
class TERRESTRE_API UItemEntityFactory : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Item Factory")
	static AEntityItem* SpawnEntityItem(const UObject* WorldContext, const FTransform SpawnTransform, const int32 ItemCount, const int32 ItemID);

};
