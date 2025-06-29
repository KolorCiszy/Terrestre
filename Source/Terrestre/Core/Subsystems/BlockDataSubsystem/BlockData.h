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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data")
	int32 DroppedItemID;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block data")
	bool bIsOpaque = true;

	int32 GetFaceTextureIndex(EDirections direction) const
	{
		if (bHasSingleTexture)
		{
			return TextureIndexUP;
		}
		switch (direction)
		{
			case EDirections::Up: return TextureIndexUP;
		break;
			case EDirections::Down: return TextureIndexDOWN;
				break;
			case EDirections::Left: return TextureIndexLEFT;
				break;
			case EDirections::Right: return TextureIndexRIGHT;
				break;
			case EDirections::Forward: return TextureIndexFORWARD;
				break;
			case EDirections::Backward: return TextureIndexBACKWARD;
				break;

		}
		return TextureIndexUP;
	}
	
};






