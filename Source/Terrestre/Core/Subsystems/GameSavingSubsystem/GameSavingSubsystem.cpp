#include "GameSavingSubsystem.h"
#include "Terrestre/Core/Gamemode/TerrestrePlayerState.h"
#include "Terrestre/Core/Character/Player/PlayerData.h"
#include "Misc/Crc.h"
#include "Serialization/BufferArchive.h"

void UGameSavingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FindAllPlayerCharactersData();
	LoadAllSavedWorlds();
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
	
	const FString FileName(playerStateToSave->GetPlayerName() + SavedPlayerCharacterFileExt);
	const FString AbsolutePath(PlayerDataSavePath + FileName);
	FPlayerData data;
	data.Name = (playerStateToSave->GetPlayerName());
	data.Transform = playerStateToSave->GetPawn()->GetActorTransform();
	FBufferArchive toBinary;
	toBinary << data;
	return FFileHelper::SaveArrayToFile(toBinary, *AbsolutePath);
	
}
bool UGameSavingSubsystem::SaveWorldData(UWorldSaveData* toSave)
{
	const FString FileName{ toSave->WorldName + WorldSaveDataFileExt };
	const FString AbsolutePath{ WorldDataSavePath + "/" + toSave->WorldName + "/" + FileName};
	FBufferArchive toBinary;
	toBinary << *toSave;
	return FFileHelper::SaveArrayToFile(toBinary, *AbsolutePath);
}
bool UGameSavingSubsystem::IsValidSavedWorldData(UWorldSaveData* toCheck)
{
	
	return DoesSavedWorldExist(toCheck->WorldName) && !toCheck->WorldName.IsEmpty() && !toCheck->CreationDate.ToString().IsEmpty();
}
bool UGameSavingSubsystem::DoesSavedWorldExist(const FString& WorldName)
{
	const FString FileName{ WorldName + WorldSaveDataFileExt };
	const FString WorldFolderPath{ WorldDataSavePath + WorldName };
	const FString WorldSaveDataPath = WorldFolderPath + '/' + FileName;
	return IFileManager::Get().DirectoryExists(*WorldFolderPath) && IFileManager::Get().FileExists(*WorldSaveDataPath);
}
void UGameSavingSubsystem::LoadAllSavedWorlds()
{
	const FString FilesToFind = "*" + WorldSaveDataFileExt;
	TArray<FString> FoundFiles;
	IFileManager::Get().FindFilesRecursive(FoundFiles, *WorldDataSavePath, *FilesToFind, true, true, false);
	
	TArray<uint8> BinaryData{};
	SavedWorlds.Reset(FoundFiles.Num());
	BinaryData.SetNumUninitialized(sizeof(UWorldSaveData));
	
	for (const FString& FileName : FoundFiles)
	{
		
		FFileHelper::LoadFileToArray(BinaryData, *FileName);
		FMemoryReader FromBinaryFile = FMemoryReader(BinaryData);
		FromBinaryFile.Seek(0);
		UWorldSaveData* LoadedData = NewObject<UWorldSaveData>();
		FromBinaryFile << *LoadedData;
		if (IsValidSavedWorldData(LoadedData) && !SavedWorlds.Contains(LoadedData))
		{
			SavedWorlds.Add(LoadedData);
		}
		
		BinaryData.Empty(sizeof(UWorldSaveData));
	}
	
}
bool UGameSavingSubsystem::DeleteSavedWorld(const FString& WorldName)
{
	const FString WorldFolderPath{ WorldDataSavePath + WorldName };
	return IFileManager::Get().DeleteDirectory(*WorldFolderPath, true,true);
}
FString UGameSavingSubsystem::GetRegionDataSavePath(FString worldName)
{
	return WorldDataSavePath + worldName + "/Region";
}
void UGameSavingSubsystem::RefreshSavedWorlds()
{
	LoadAllSavedWorlds();
}
void UGameSavingSubsystem::SaveChunkRegion(FChunkRegion& region, FIntVector regionID)
{
	FBufferArchive bufAr{};
	bufAr << region;
	const FString FileName = FString::Format(TEXT("{0}.{1}.{2}.{3}"), { regionID.X, regionID.Y, regionID.Z, RegionFileExt });
	
}
