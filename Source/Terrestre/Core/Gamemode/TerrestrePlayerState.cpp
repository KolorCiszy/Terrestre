#include "TerrestrePlayerState.h"
#include "Terrestre/Core/Character/Player/PlayerData.h"

ATerrestrePlayerState::ATerrestrePlayerState()
{

	
}
void ATerrestrePlayerState::SetSpawnLocation(FVector location)
{
	SpawnTransform.SetLocation(location);
}
FVector ATerrestrePlayerState::GetSpawnLocation() const
{
	return SpawnTransform.GetLocation();
}
FTransform ATerrestrePlayerState::GetSpawnTransform() const
{
	return SpawnTransform;
}
void ATerrestrePlayerState::ReadLoadedPlayerData(TSharedPtr<FPlayerData> data)
{
	check(data != nullptr);
	SpawnTransform = data.Get()->Transform;
	SetPlayerName(MoveTemp(data.Get()->Name));
}