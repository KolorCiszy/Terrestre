#pragma once

#include "CoreMinimal.h"
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
	
	static inline const FString PlayerDataSavePath = FPaths::ProjectDir() + TEXT("Saved/SaveGames/Players/");
 
	static inline const FString SavedPlayerCharacterFileExt = TEXT("dat");

	static inline const FString WorldDataSavePath = FPaths::ProjectDir() + TEXT("Saved/SaveGames/Worlds/");

	void FindAllPlayerCharactersData();

	
	/* Loads player data from disk, returns nullptr if load fails */
	
	TSharedPtr<FPlayerData> LoadPlayerData(const FString& playerName);
	
	/* Saves player data to disk, returns true if succesful, false otherwise */
	UFUNCTION(BlueprintCallable, Category = "Game Saving Subsystem")
	bool SavePlayerData(ATerrestrePlayerState* playerToSave);

	/* Checks if there is some data saved on disk that corresponds to this player name */
	UFUNCTION(BlueprintPure, Category = "Game Saving Subsystem")
	bool DoesPlayerDataExist(const FString& playerName) const;
};