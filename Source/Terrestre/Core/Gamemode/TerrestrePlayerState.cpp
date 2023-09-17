#include "TerrestrePlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Terrestre/Core/Subsystems/GameSavingSubsystem.h"
#include "Terrestre/Core/Player/PlayerData.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"

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