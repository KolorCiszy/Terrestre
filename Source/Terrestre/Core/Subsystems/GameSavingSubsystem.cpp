#include "GameSavingSubsystem.h"
#include "Terrestre/Core/Gamemode/TerrestrePlayerState.h"
#include "Terrestre/Core/Character/Player/PlayerData.h"
#include "Serialization/BufferArchive.h"

void UGameSavingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FindAllPlayerCharactersData();
}
void UGameSavingSubsystem::Deinitialize()
{
	/*
	const FString name = "Jorge";
	const FString FileName(name + "." + SavedPlayerCharacterFileExt);
	const FString AbsolutePath(PlayerDataSavePath + FileName);
	FPlayerData data;
	data.Name = "Jorge";
	data.Transform = FTransform{};
	FBufferArchive toBinary;
	toBinary << data;
	FFileHelper::SaveArrayToFile(toBinary, *AbsolutePath);
	*/
	
}
void UGameSavingSubsystem::FindAllPlayerCharactersData()
{
	IFileManager::Get().FindFiles(PlayerCharactersFoundOnDisk, *PlayerDataSavePath, *SavedPlayerCharacterFileExt);
}

bool UGameSavingSubsystem::DoesPlayerDataExist(const FString& playerName) const
{
	const FString FileName(playerName + "." + SavedPlayerCharacterFileExt);
	const FString AbsolutePath(PlayerDataSavePath + FileName);
	return FPaths::FileExists(*AbsolutePath);
}
TSharedPtr<FPlayerData> UGameSavingSubsystem::LoadPlayerData(const FString& playerName)
{
	const FString FileName(playerName + "." + SavedPlayerCharacterFileExt);
	const FString AbsolutePath(PlayerDataSavePath + FileName);
	TArray<uint8> fromBinary;
	TSharedPtr<FPlayerData> data = MakeShared<FPlayerData>();
	if (FFileHelper::LoadFileToArray(fromBinary, *AbsolutePath))
	{
		FMemoryReader FromBinaryFile = FMemoryReader(fromBinary);
		FromBinaryFile.Seek(0);
		FromBinaryFile << *data;
		return data;
	};
	return nullptr;

}
bool UGameSavingSubsystem::SavePlayerData(ATerrestrePlayerState* playerStateToSave)
{
	
	const FString FileName(playerStateToSave->GetPlayerName() + "." + SavedPlayerCharacterFileExt);
	const FString AbsolutePath(PlayerDataSavePath + FileName);
	FPlayerData data;
	data.Name = (playerStateToSave->GetPlayerName());
	data.Transform = playerStateToSave->GetPawn()->GetActorTransform();
	FBufferArchive toBinary;
	toBinary << data;
	if (FFileHelper::SaveArrayToFile(toBinary, *AbsolutePath))
	{
		return true;
	}
	else
	{
		return false;
	}
}