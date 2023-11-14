#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataRegistry/Public/DataRegistrySubsystem.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/Misc/Directions.h"
#include "BlockData.generated.h"


UENUM(BlueprintType)
enum class EBlockMeshType : uint8
{
	/* This block's mesh is a standard, full cube */
	FULL,
	/* This block has no mesh */
	NONE,

	COUNT

};


USTRUCT(BlueprintType)
struct FBlockData : public FTableRowBase
{

	GENERATED_BODY()

	/* Diplay name of a block*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data")
	FName DisplayName = TEXT("NONE");

	/* True if block has different textures on its sides, e.g. grass, if false, the UP index is used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data")
	bool bHasSingleTexture = true;

	/* Index of texture in Terrain texture array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data")
	int32 TextureIndexUP = 0;

	/* Index of texture in Terrain texture array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data", 
				meta = (EditCondition = "bHasSingleTexture == false", EditConditionHides))
	int32 TextureIndexDOWN = 0;

	/* Index of texture in Terrain texture array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data",
				meta = (EditCondition = "bHasSingleTexture == false", EditConditionHides))
	int32 TextureIndexLEFT = 0;

	/* Index of texture in Terrain texture array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data",
				meta = (EditCondition = "bHasSingleTexture == false", EditConditionHides))
	int32 TextureIndexRIGHT = 0;

	/* Index of texture in Terrain texture array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data",
				meta = (EditCondition = "bHasSingleTexture == false", EditConditionHides))
	int32 TextureIndexFORWARD = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data",
				meta = (EditCondition = "bHasSingleTexture == false", EditConditionHides))
	int32 TextureIndexBACKWARD = 0;

	/* Index of texture in Terrain texture array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data")
	EBlockMeshType MeshType = EBlockMeshType::NONE;
};

enum class EDirections;
namespace BlockData
{
	inline FName const RegistryName = TEXT("BlockData");
	
	bool bInitialized = false;
	
	inline UDataRegistry* dataRegistry;

	static void Initialize()
	{
		if(!bInitialized)
		{
			dataRegistry = UDataRegistrySubsystem::Get()->GetRegistryForType(BlockData::RegistryName);
		}
	}

	static const FBlockData* GetBlockData(int64 blockID)
	{
		FName name{ *FString::FromInt(blockID) };
		FDataRegistryId blockDataID = FDataRegistryId(BlockData::RegistryName, name);
		return UDataRegistrySubsystem::Get()->GetCachedItem<FBlockData>(FDataRegistryId(RegistryName, name));
	}
	static int32 GetBlockTextureIndex(int64 blockID, EDirections blockSide)
	{
		auto blockData = GetBlockData(blockID);

		if(blockData)
		{
			if(blockData->bHasSingleTexture)
			{
				return blockData->TextureIndexUP;
			}
			switch (blockSide)
			{
				case EDirections::Forward: return blockData->TextureIndexFORWARD;
					break;
				case EDirections::Backward: return blockData->TextureIndexBACKWARD;
					break;
				case EDirections::Left: return blockData->TextureIndexLEFT;
					break;
				case EDirections::Right: return blockData->TextureIndexRIGHT;
					break;
				case EDirections::Up: return blockData->TextureIndexUP;
					break;
				case EDirections::Down: return blockData->TextureIndexDOWN;
					break;
			}	
		}
		return -1;
	}
	static EBlockMeshType GetBlockMeshType(int64 blockID)
	{
		auto blockData = GetBlockData(blockID);
		if (blockData)
		{
			return blockData->MeshType;
		}
		return EBlockMeshType::NONE;
	}
	static FName GetBlockDisplayName(int64 blockID)
	{
		auto blockData = GetBlockData(blockID);
		if (blockData)
		{
			return blockData->DisplayName;
		}
		return TEXT("NONE");
	}
	
}





