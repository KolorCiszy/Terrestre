#include "ItemEntityFactory.h"

#include "ItemData.h"
#include "ItemDataSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Terrestre/Core/Entity/EntityItem.h"

AEntityItem* UItemEntityFactory::SpawnEntityItem(const UObject* WorldContext, const FTransform SpawnTransform, const int32 ItemCount, const int32 ItemID)
{
	const auto World = WorldContext->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnEntityItem: WorldContext is null"));
	}
	auto ItemData = GEngine->GetEngineSubsystem<UItemDataSubsystem>()->GetItemData(ItemID);
	if (!ItemData)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnEntityItem: ItemData is null"));
	}
	TSubclassOf<AEntityItem> SpawnActorClass = ItemData->SpawnEntityClass.LoadSynchronous();
	SpawnActorClass = SpawnActorClass == nullptr ? TSubclassOf<AEntityItem>(AEntityItem::StaticClass()) : SpawnActorClass;

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const auto SpawnedActor = World->SpawnActorDeferred<AEntityItem>(SpawnActorClass, SpawnTransform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	SpawnedActor->SetSpawnParams(ItemCount, ItemID);
	UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnTransform);
	if (!SpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnEntityItem: Failed to spawn entity"));
		return nullptr;
	}
	return SpawnedActor;
}
