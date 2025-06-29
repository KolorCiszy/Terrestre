#pragma once

#include "CoreMinimal.h"
#include "WorldSaveData.h"
#include "Terrestre/Core/Chunk/Storage/ChunkRegion.h"
#include "GameSavingSubsystem.generated.h"


class ATerrestrePlayerState;
struct FPlayerData;
UCLASS()
class UGameSavingSubsystem : public UGameInstanceSubsystem
{
public:
	GENERATED_BODY()
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> PlayerCharactersFoundOnDisk;
	
	/* ProjectDir/Saved/SaveGames/Players/ */
	static inline const FString PlayerDataSavePath = FPaths::ProjectDir() + TEXT("Saved/SaveGames/Players/");
 
	/* .dat */
	static inline const FString SavedPlayerCharacterFileExt = TEXT(".dat");

	/* ProjectDir/Saved/SaveGames/Worlds/ */
	static inline const FString WorldDataSavePath = FPaths::ProjectDir() + TEXT("Saved/SaveGames/Worlds/");

	/* .reg */
	static inline const FString RegionFileExt = TEXT(".reg");

	/* .wd */
	static inline const FString WorldSaveDataFileExt = TEXT(".wd");

	void FindAllPlayerCharactersData();
	/* Gets all saved worlds from disk prior returning */
	UFUNCTION(BlueprintPure, Category="Game Saving Subsystem")
	TArray<UWorldSaveData*>& GetSavedWorlds()
	{
		RefreshSavedWorlds();
		return SavedWorlds;	
	}
	UFUNCTION(BlueprintCallable, Category = "Game Saving Subsystem")
	void RefreshSavedWorlds();

	UFUNCTION(BlueprintCallable, Category="Game Saving Subsystem")
	bool SaveWorldData(UWorldSaveData* toSave); // check if world is already saved

	UFUNCTION(BlueprintPure, Category="Game Saving Subsystem")
	bool DoesSavedWorldExist(const FString& WorldName);

	UFUNCTION(BlueprintPure, Category = "Game Saving Subsystem")
	bool IsValidSavedWorldData(UWorldSaveData* toCheck);	

	UFUNCTION(BlueprintCallable, Category = "Game Saving Subsystem")
	bool DeleteSavedWorld(const FString& WorldName);

	/* Loads player data from disk, returns nullptr if load fails */
	TSharedPtr<FPlayerData> LoadPlayerData(const FString& playerName);
	
	/* Saves player data to disk, returns true if succesful, false otherwise */
	UFUNCTION(BlueprintCallable, Category = "Game Saving Subsystem")
	bool SavePlayerData(ATerrestrePlayerState* playerToSave);

	/* Checks if there is some data saved on disk that corresponds to this player name */
	UFUNCTION(BlueprintPure, Category = "Game Saving Subsystem")
	bool DoesPlayerDataExist(const FString& playerName) const;

	void SaveChunkRegion(FChunkRegion& region, FIntVector regionID);
	/*
		/* Sets the current open world at the start of game, if the name is already set it has to be reset before assigning again
	UFUNCTION(BlueprintCallable, Category = "Game Saving Subsystem")
	bool SetCurrentOpenWorld(const FString& WorldName);
	*/
private:
	UPROPERTY()
	TArray<UWorldSaveData*> SavedWorlds;


	FString GetRegionDataSavePath(FString worldName);

	void LoadAllSavedWorlds();
};