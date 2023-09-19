// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrestreGameModeBase.h"
#include "Terrestre/Core/Gamemode/TerrestrePlayerState.h"
#include "Terrestre/Core/Player/PlayerCharacter.h"
#include "Terrestre/Core/Subsystems/GameSavingSubsystem.h"

ATerrestreGameModeBase::ATerrestreGameModeBase()
{
	DefaultPlayerName = FText::FromString(L"Jorge");
}
void ATerrestreGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Game mode BeginPlay"))
}
void ATerrestreGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	NewPlayer->SetName(DefaultPlayerName.ToString());
	auto newPlayerState = Cast<ATerrestrePlayerState>(NewPlayer->PlayerState);
	newPlayerState->SetPlayerName(DefaultPlayerName.ToString());
	auto playerName = newPlayerState->GetPlayerName();
	UE_LOG(LogTemp, Warning, TEXT("Player State Valid"))
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
			FVector spawnLocation = { 0, 0, 31000 };//UChunkUtilityLib::GetChunkManager()->DefaultPlayerSpawnLocation;
			UE_LOG(LogTemp, Warning, TEXT("Player State spawn Location: %s"), *spawnLocation.ToString())
			newPlayerState->SetSpawnLocation(spawnLocation);
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

APlayerCharacter* ATerrestreGameModeBase::SpawnPlayerCharacter(APlayerState* owner)
{
	FTransform spawnTransform = Cast<ATerrestrePlayerState>(owner)->GetSpawnTransform();
	FActorSpawnParameters params{};
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	auto player = GetWorld()->SpawnActor<APlayerCharacter>(PlayerCharacterClass,spawnTransform, params);
	auto owningController = Cast<APlayerController>(owner->GetOwningController());
	owningController->UnPossess();
	owningController->SetPawn(player);
	return player;
}