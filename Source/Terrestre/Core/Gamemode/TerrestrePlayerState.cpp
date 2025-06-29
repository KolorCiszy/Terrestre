#include "TerrestrePlayerState.h"
#include "Terrestre/Core/Character/Player/PlayerData.h"
#include "Terrestre/Core/Components/InventoryComponent.h"

ATerrestrePlayerState::ATerrestrePlayerState()
{
	MainInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Main Inventory"));
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	BlockPlaceCooldown = 0.5f;
	AttackCooldown = 1.0f;
}
void ATerrestrePlayerState::SetSpawnLocation(FVector location)
{
	SpawnTransform.SetLocation(location);
}

FTransform ATerrestrePlayerState::GetSpawnTransform() const
{
	return SpawnTransform;
}
void ATerrestrePlayerState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentBlockPlaceCooldown = FMath::Clamp(CurrentBlockPlaceCooldown - DeltaTime, 0, BlockPlaceCooldown);
	CurrentAttackCooldown = FMath::Clamp(CurrentAttackCooldown - DeltaTime, 0, AttackCooldown);
}
void ATerrestrePlayerState::ReadLoadedPlayerData(TSharedPtr<FPlayerData> data)
{
	check(data != nullptr);
	SpawnTransform = data.Get()->Transform;
	SetPlayerName(MoveTemp(data.Get()->Name));
}