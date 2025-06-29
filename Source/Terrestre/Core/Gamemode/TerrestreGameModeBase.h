// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Terrestre/Core/Gamemode/TerrestreGameModeInterface.h"
#include "TerrestreGameModeBase.generated.h"



class APlayerCharacter;
class AChunkManager;
class ARegionManager;
class ATerrestrePlayerState;
class UTerrainShaperSettings;
/**
 * 
 */
UCLASS(Config=Game)
class TERRESTRE_API ATerrestreGameModeBase : public AGameModeBase, public ITerrestreGameModeInterface
{
	GENERATED_BODY()

public:
	ATerrestreGameModeBase();

	virtual bool RequestPlayerPawnSpawn(APlayerController* PlayerController) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<APlayerCharacter> PlayerCharacterClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk Classes")
	TSubclassOf<AChunkManager> ChunkManagerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk Classes")
	TSubclassOf<ARegionManager> RegionManagerClass;



	/* How many chunks will be fully visible from the chunk player is currently in*/
	UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Chunk rendering")
	uint8 RenderDistance;

	/* How many regions will the game pre load from the region the player is currently in, all of them will have at least the first step of generation complete */
	UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Chunk region data")
	uint8 RegionLoadDistance;


	/* How many chunks can be spawned during one chunk manager tick */
	UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Chunk manager")
	int32 ChunksToSpawnPerTick;

	/* How many chunks can be despawned during one chunk manager tick */
	UPROPERTY(EditDefaultsOnly, GlobalConfig, BlueprintReadOnly, Category = "Chunk manager")
	int32 ChunksToDespawnPerTick;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	FVector DefaultPlayerSpawnLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	FVector DefaultSpawnChunkStartLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk manager")
	int32 SpawnChunkRadius;


protected:
	virtual void BeginPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;

	virtual void EndPlay(EEndPlayReason::Type reason) override;
};
