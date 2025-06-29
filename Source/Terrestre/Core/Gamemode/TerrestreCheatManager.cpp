// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Gamemode/TerrestreCheatManager.h"
#include "TerrestrePlayerState.h"
#include "Logging/StructuredLog.h"

void UTerrestreCheatManager::gm(EPlayerGameMode newGameMode)
{
	if (auto Controller = GetOuterAPlayerController())
	{
		if (auto PlayerState = Controller->GetPlayerState<ATerrestrePlayerState>())
		{
			PlayerState->SetPlayerGameMode(newGameMode);
			
			UE_LOGFMT(LogTemp, Log, "Changed {0} GameMode to {1}", PlayerState->GetPlayerName(), UEnum::GetValueAsString(newGameMode));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerState is nullptr or not ATerrestrePlayerState"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is nullptr"));
	}
	
}
