#include "ItemDataSubsystem.h"
#include "ItemData.h"
#include "DataRegistrySubsystem.h"
void UItemDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	
}
void UItemDataSubsystem::Deinitialize()
{

}
const FItemData* UItemDataSubsystem::GetItemData(int32 itemID) const // change item ID to FName?
{
	const FName name{ *FString::FromInt(itemID) };
	FDataRegistryId itemDataID = FDataRegistryId(RegistryName, name);
	return UDataRegistrySubsystem::Get()->GetCachedItem<FItemData>(itemDataID);
}
void UItemDataSubsystem::GetItemData_BP(int32 itemID, FItemData& outData) const
{
	outData = *GetItemData(itemID);
}