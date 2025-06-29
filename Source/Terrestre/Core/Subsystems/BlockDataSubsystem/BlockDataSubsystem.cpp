#include "BlockDataSubsystem.h"
#include "DataRegistrySubsystem.h"
#include "Terrestre/Core/Chunk/Misc/Directions.h"
#include "BlockData.h"


void UBlockDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	
}
void UBlockDataSubsystem::Deinitialize()
{
	
}
const FBlockData* UBlockDataSubsystem::GetBlockData(int64 blockID) const // change block ID to name?
{
	const FName name{ *FString::FromInt(blockID) };
	FDataRegistryId blockDataID = FDataRegistryId(RegistryName, name);
	return UDataRegistrySubsystem::Get()->GetCachedItem<FBlockData>(blockDataID);
}
void UBlockDataSubsystem::GetBlockData_BP(int64 blockID, FBlockData& outData) const
{
	outData = *GetBlockData(blockID);
}

