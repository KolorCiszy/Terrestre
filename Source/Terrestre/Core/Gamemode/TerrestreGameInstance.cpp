// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Gamemode/TerrestreGameInstance.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Kismet/GameplayStatics.h"



void UTerrestreGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UTerrestreGameInstance::ReceivePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UTerrestreGameInstance::ReceivePostLoadMapWithWorld);

	SetupWorkerThreadPools();
	

}
void UTerrestreGameInstance::ReceivePreLoadMap(const FString& InMapName)
{
	
	
}
void UTerrestreGameInstance::ReceivePostLoadMapWithWorld(UWorld* InLoadedWorld)
{
	
}
void UTerrestreGameInstance::UnloadWorld()
{

}
void UTerrestreGameInstance::LoadWorld(UWorldSaveData* WorldToLoad)
{
	UGameplayStatics::OpenLevel(this, PrimaryDimensionName, true);
	OpenWorldData = WorldToLoad;
}
void UTerrestreGameInstance::SetChunkManagerTickEnabled(bool bEnabled) 
{
	/*
	if(chunkManager)
	{
		chunkManager->SetTickEnabled(bEnabled);
	}
	*/
}
void UTerrestreGameInstance::SetRenderDistance(uint8 newDistance)
{
	/*
	if (auto chunkManager = UChunkUtilityLib::GetChunkManager())
	{
		chunkManager->SetRenderDistance(newDistance);
	}
	*/
}
void UTerrestreGameInstance::SetupWorkerThreadPools()
{
	
	ensureMsgf(ChunkMeshingThreadCount < FGenericPlatformMisc::NumberOfCoresIncludingHyperthreads() &&
			ChunkMeshingThreadCount > 0,
			TEXT("That's an invalid amount of threads to create"));
	
	
	ChunkMeshingTP = FQueuedThreadPool::Allocate();

	if (ChunkMeshingTP)
	{
		ChunkMeshingTP->Create(ChunkMeshingThreadCount, 128 * 1024, EThreadPriority::TPri_Highest, TEXT("Chunk Meshing Thread Pool"));
	}
	
	
	
}
void UTerrestreGameInstance::Shutdown()
{
	Super::Shutdown();
	ChunkMeshingTP->Destroy();
	delete ChunkMeshingTP;
}