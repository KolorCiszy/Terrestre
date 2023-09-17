// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Gamemode/TerrestreGameInstance.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"



void UTerrestreGameInstance::Init()
{
	Super::Init();
}

void UTerrestreGameInstance::SetChunkManagerTickEnabled(bool bEnabled) 
{
	auto chunkManager = UChunkUtilityLib::GetChunkManager();
	if(chunkManager)
	{
		chunkManager->SetTickEnabled(bEnabled);
	}
}
void UTerrestreGameInstance::SetRenderDistance(uint8 newDistance)
{
	if (auto chunkManager = UChunkUtilityLib::GetChunkManager())
	{
		chunkManager->SetRenderDistance(newDistance);
	}
}
