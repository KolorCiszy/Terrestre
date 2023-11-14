// Fill out your copyright notice in the Description page of Project Settings.

#include "Terrestre.h"
#include "Terrestre/Core/TerrestreNativeGameplayTags.h"
#include "GameplayTagsManager.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, Terrestre, "Terrestre" );

void FTerrestreGameModule::StartupModule()
{
	UGameplayTagsManager::Get().AddNativeGameplayTag("Terrestre.Character.Player");
	UGameplayTagsManager::Get().DoneAddingNativeTags();
}