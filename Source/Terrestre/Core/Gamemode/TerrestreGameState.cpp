// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Gamemode/TerrestreGameState.h"
#include "TerrestreGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Terrestre/Core/Chunk/Storage/RegionManager.h"

ATerrestreGameState::ATerrestreGameState()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ATerrestreGameState::ReceivePostLoadMapWithWorld);
}
void ATerrestreGameState::ReceivePostLoadMapWithWorld(UWorld* InLoadedWorld)
{


}
AChunkManager* ATerrestreGameState::GetChunkManager()
{
	if (!ChunkManager)
	{
		ATerrestreGameModeBase* GameMode = CastChecked<ATerrestreGameModeBase>(GetWorld()->GetAuthGameMode());

		AActor* L_ChunkManager = UGameplayStatics::GetActorOfClass(GetWorld(), GameMode->ChunkManagerClass);

		ChunkManager = CastChecked<AChunkManager>(L_ChunkManager);
	}
	return ChunkManager;
}
void ATerrestreGameState::BeginPlay()
{
	Super::BeginPlay();
	ATerrestreGameModeBase* GameMode = CastChecked<ATerrestreGameModeBase>(GetWorld()->GetAuthGameMode());

	if (!ChunkManager)
	{
		auto World = GetWorld();
		AActor* ChunkManagerActor = World->SpawnActor(GameMode->ChunkManagerClass);
		ChunkManager = CastChecked<AChunkManager>(ChunkManagerActor);
	}
	if (!RegionManager)
	{
		AActor* RegionManagerActor = GetWorld()->SpawnActor(GameMode->RegionManagerClass);
		RegionManager = CastChecked<ARegionManager>(RegionManagerActor);
	}
}

