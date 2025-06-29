#pragma once

#include "CoreMinimal.h"
#include "ItemDataSubsystem.generated.h"

class UDataRegistry;
struct FItemData;
/* An Engine Subsystem for accesing item data */
UCLASS()
class UItemDataSubsystem : public UEngineSubsystem
{
public:

	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	
	const FItemData* GetItemData(int32 itemID) const;

	UFUNCTION(BlueprintPure, Category = "Item Data Subsytem")
	void GetItemData_BP(int32 itemID, FItemData& outData) const;

private:

	const FName RegistryName = TEXT("ItemData");

	
};