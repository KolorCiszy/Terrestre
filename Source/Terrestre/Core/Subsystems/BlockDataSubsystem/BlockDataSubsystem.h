#pragma once

#include "CoreMinimal.h"
#include "BlockDataSubsystem.generated.h"

class UDataRegistry;
struct FBlockData;
/* An Engine Subsystem for accesing block data */
UCLASS()
class UBlockDataSubsystem : public UEngineSubsystem
{
public:
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	
	const FBlockData* GetBlockData(int64 blockID) const;

	UFUNCTION(BlueprintPure, Category = "Block Data Subsytem")
	void GetBlockData_BP(int64 blockID, FBlockData& outData) const;

private:
	const FName RegistryName = TEXT("BlockData");

	
};