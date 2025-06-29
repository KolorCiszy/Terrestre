// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrestreGameModeBase.h"
#include "Terrestre/Core/Gamemode/TerrestrePlayerState.h"
#include "Terrestre/Core/Character/Player/PlayerCharacter.h"
#include "Terrestre/Core/Subsystems/GameSavingSubsystem/GameSavingSubsystem.h"
#include "TerrestreGameState.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "TerrestrePlayerController.h"

ATerrestreGameModeBase::ATerrestreGameModeBase()
{
	DefaultPlayerName = FText::FromString(L"Jorge");
	DefaultPlayerSpawnLocation = FVector{ 50,50,10 };
	ChunksToDespawnPerTick = 2;
	ChunksToSpawnPerTick = 10;
}
void ATerrestreGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Game mode BeginPlay"))
	if(RenderDistance < SpawnChunkRadius)
	{
		UE_LOG(LogTemp, Error, TEXT("Render distance is smaller than spawn chunk radius, this will cause chunks to not be spawned correctly!"))
	}
	
}
void ATerrestreGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	NewPlayer->SetName(DefaultPlayerName.ToString());
	auto newPlayerState = Cast<ATerrestrePlayerState>(NewPlayer->PlayerState);
	newPlayerState->SetPlayerName(DefaultPlayerName.ToString());
	auto playerName = newPlayerState->GetPlayerName();
	UE_LOG(LogTemp, Warning, TEXT("Player State Valid"))

		// Check if the player data exists on disk, if it does, load it
	if (UGameInstance* gameInstance = GetGameInstance())
	{
		auto GSS = gameInstance->GetSubsystem<UGameSavingSubsystem>();	// Game Saving Subsystem

		if (GSS->DoesPlayerDataExist(playerName))
		{
			TSharedPtr<FPlayerData> loadedData = GSS->LoadPlayerData(playerName);
			newPlayerState->ReadLoadedPlayerData(loadedData);
		}
		else
		{
			// If player data does not exist, create a new one
			newPlayerState->SetSpawnLocation(DefaultPlayerSpawnLocation);
		}
	}
	
}
void ATerrestreGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	/*
	if (UGameInstance* gameInstance = GetGameInstance())
	{
		auto GSS = gameInstance->GetSubsystem<UGameSavingSubsystem>();	// Game Saving Subsystem

		GSS->SavePlayerData(Exiting->GetPlayerState<ATerrestrePlayerState>());
	}
	*/
}

void ATerrestreGameModeBase::EndPlay(EEndPlayReason::Type reason)
{
	
}


bool ATerrestreGameModeBase::RequestPlayerPawnSpawn(APlayerController* PlayerController)
{
	auto PlayerState = PlayerController->GetPlayerState<ATerrestrePlayerState>();
	if (PlayerState == nullptr)
	{
		UE_LOG(LogPlayerController, Error, TEXT("Player State is null in RequestPlayerPawnSpawn"));
		return false;
	}
	FVector SpawnLocation = PlayerState->GetSpawnTransform().GetLocation();

	auto ChunkManager = GetGameState<ATerrestreGameState>()->GetChunkManager();

	if (ChunkManager == nullptr)
	{
		UE_LOG(LogPlayerController, Error, TEXT("Chunk Manager is null in RequestPlayerPawnSpawn"));
		return false;
	}
	FVector SpawnLocationAdjusted = ChunkManager->AdjustPlayerSpawnLocation(SpawnLocation);

	FActorSpawnParameters SpawnParams{};
	SpawnParams.Owner = PlayerController;

	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<APlayerCharacter>(PlayerCharacterClass, SpawnLocationAdjusted, PlayerState->GetSpawnTransform().Rotator(), SpawnParams);
	return true;
}
