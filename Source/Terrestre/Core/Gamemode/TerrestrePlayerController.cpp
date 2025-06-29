// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Gamemode/TerrestrePlayerController.h"

void ATerrestrePlayerController::SetSelectedHotbarSlot(uint8 index)
{
	if (SelectedHotbarSlot != index)
	{
		SelectedHotbarSlot = index;
		OnUpdateHotbarWidget_Implemenetation();
	}
}
uint8 ATerrestrePlayerController::GetSelectedHotbarSlot()
{
	return SelectedHotbarSlot;
}
bool ATerrestrePlayerController::IsHotbarSlotSelected(uint8 index)
{
	return SelectedHotbarSlot == index;
}
void ATerrestrePlayerController::OnUpdateHotbarWidget_Implemenetation()
{
	OnUpdateHotbarWidget();
}
