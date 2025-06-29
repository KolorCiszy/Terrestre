#include "TerrainSurfaceDecorator.h"
#include "TerrainShaper.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/Storage/ChunkRegion.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Terrestre/Core/Chunk/Storage/RegionManager.h"
#include <Terrestre/Core/Subsystems/WorldGenSubsystem/WorldGenSubsystem.h>
#include <Terrestre/Core/Gamemode/TerrestreGameInstance.h>
#include "TerrainSurfaceDecoratorSettings.h"


void UTerrainSurfaceDecorator::Initialize()
{
	if (const UWorld* World = GetWorld())
	{
		const UTerrestreGameInstance* GameInstance = World->GetGameInstanceChecked<UTerrestreGameInstance>();
		if (GameInstance)
		{
			Settings = GameInstance->TerrainSurfaceDecoratorSettings;
		}
		else
		{
			checkf(0, TEXT("No Surface Decorator Settings set in game instance"));
		}
	}
}

bool UTerrainSurfaceDecorator::GenerateSurfaceLayer(AChunk* Chunk, FChunkData& DataCopy)
{
	if (DataCopy.BlockPalette.IsEmpty())
	{
		return true;
	}
	auto UpChunk = Chunk-> GetNeighbourChunk(EDirections::Up);

	int32 TimeoutCounter = 0;
	while(!UpChunk)
	{
		FPlatformProcess::Sleep(0.1f);
		UpChunk = Chunk->GetNeighbourChunk(EDirections::Up);
		if (TimeoutCounter > 20)
		{
			UE_LOG(LogTemp, Warning, TEXT("Timeout while waiting for UpChunk to be ready in GenerateSurfaceLayer"));
			return false;
		}
		TimeoutCounter++;

	}
	TimeoutCounter = 0;
	while(!UpChunk->IsTerrainShapeDataReady())
	{
		FPlatformProcess::Sleep(0.1f);
		if (TimeoutCounter > 20)
		{
			UE_LOG(LogTemp, Warning, TEXT("Timeout while waiting for UpChunk data to be ready in GenerateSurfaceLayer"));
			return false;
		}
		TimeoutCounter++;
	}
	auto UpChunkDataCopy = UpChunk->GetChunkData();

	FIntVector ChunkBlockLocation = UChunkUtilityLib::WorldLocationToBlockPos(Chunk->GetActorLocation());

	float SeaLevel = GetWorld()->GetGameInstance<UTerrestreGameInstance>()->TerrainShaperSettings->SeaLevel;
	int32 SubsBlocksBelowHeight = GetWorld()->GetGameInstance<UTerrestreGameInstance>()->TerrainShaperSettings->SubsurfaceBlocksBelowHeight;

	for (int32 x{}; x < FChunkConstants::Size; x++)
	{
		for (int32 y{}; y < FChunkConstants::Size; y++)
		{
			FBlockState UpBlock = UpChunkDataCopy.BlockPalette.GetBlockAtIndex(UChunkUtilityLib::LocalBlockPosToIndex({ x, y, 0 }));

			int32 SubSurfaceMaterialDepth = 0;
			FBlockState SubSurfaceMaterialToPlace;
			for (int32 z = FChunkConstants::Size - 1; z >= 0; z--) // Search from top to bottom for change from solid to air block
			{
				FIntVector currentLocalPos{ x, y, z };
				int32 currentBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(currentLocalPos);
				FBlockState currentBlock = DataCopy.BlockPalette.GetBlockAtIndex(currentBlockIndex);

				if (currentBlock.blockID != TERRESTRE_BLOCK_BASALT)
				{
					SubSurfaceMaterialDepth = 0; 
					UpBlock = currentBlock;
					continue; 
				}
				int32 currentWorldZ = (ChunkBlockLocation.Z + z);
		
				if (UpBlock.IsAirBlock())
				{
					if (currentWorldZ > SeaLevel + SubsBlocksBelowHeight)
					{
						DataCopy.BlockPalette.ModifyBlockAtIndex(
						currentBlockIndex, 
						FBlockState{ TERRESTRE_BLOCK_GRASS });
                
					
						SubSurfaceMaterialDepth = Settings->DirtBlocksBelowGrass - 1;
						SubSurfaceMaterialToPlace = FBlockState{ TERRESTRE_BLOCK_DIRT };
						UpBlock = FBlockState{ TERRESTRE_BLOCK_GRASS };
					}
					else
					{
						DataCopy.BlockPalette.ModifyBlockAtIndex(
						   currentBlockIndex,
						   FBlockState{ TERRESTRE_BLOCK_SAND });

						SubSurfaceMaterialDepth = Settings->SandBlocksBelowSeaLevel - 1;
						SubSurfaceMaterialToPlace = FBlockState{ TERRESTRE_BLOCK_SAND };
						UpBlock = FBlockState{ TERRESTRE_BLOCK_SAND }; 
					}
				}
				else if (SubSurfaceMaterialDepth > 0)
				{
					DataCopy.BlockPalette.ModifyBlockAtIndex(
					   currentBlockIndex,
					   SubSurfaceMaterialToPlace);

					UpBlock = SubSurfaceMaterialToPlace; 
					SubSurfaceMaterialDepth--;
				}
				else
				{
					UpBlock = currentBlock;
				}
			}	
		}
	}
	return true;
}

bool UTerrainSurfaceDecorator::GenerateTerrainSurfaceDecorations(FChunkProtoRegion& protoRegion, FIntVector RegionID)
{	
	/*
	FIntVector regionOriginB = UChunkUtilityLib::GetRegionOrigin(RegionID);

	FIntVector UpChunkRegionID{ RegionID.X, RegionID.Y, RegionID.Z + 1 };
	FIntVector UpRegionOriginB = UChunkUtilityLib::GetRegionOrigin(UpChunkRegionID);
	FChunkRegion* UpChunkRegion = URegionManager::Get()->GetChunkRegionByID_WG(UpChunkRegionID);
	FIntVector DownChunkRegionID{ RegionID.X, RegionID.Y, RegionID.Z - 1 };
	FIntVector DownChunkRegionOriginB = UChunkUtilityLib::GetRegionOrigin(DownChunkRegionID);
	FChunkRegion* DownChunkRegion = URegionManager::Get()->GetChunkRegionByID_WG(DownChunkRegionID);
	//FChunkRegion UpRegionCopy{};
	//FChunkRegion DownRegionCopy{};
	bool bUpChunkRegionValid{};
	bool bDownChunkRegionValid{};

	if(UpChunkRegion && DownChunkRegion)
	{
		//UpRegionCopy = *UpChunkRegion;
		//DownRegionCopy = *DownChunkRegion;
		bUpChunkRegionValid = true;
		bDownChunkRegionValid = true;
		/*
		UpChunkRegion.Reset();
		DownChunkRegion.Reset();
		
	}
	else
	{
		return false;
	}
	
	
	
	for (int32 x = 0; x < FChunkRegion::RegionSizeInBlocks; x++)
	{
		int32 worldXPosB = regionOriginB.X + x;
		
		for (int32 y = 0; y < FChunkRegion::RegionSizeInBlocks; y++)
		{
			int32 worldYPosB = regionOriginB.Y + y;

			FBlockState UpperBlock = URegionManager::Get()->GetBlockStateAtWorldBlockLoc_ReadOnly({ worldXPosB, worldYPosB, UpRegionOriginB.Z });
			FBlockState UpperBlock1 = URegionManager::Get()->GetBlockStateAtWorldBlockLoc_ReadOnly({ worldXPosB, worldYPosB, UpRegionOriginB.Z + 1 });
			
			FVector UpChunkLocation = UChunkUtilityLib::BlockPosToChunkWorldLocation({ worldXPosB, worldYPosB, UpRegionOriginB.Z});
			FIntVector localPos = UChunkUtilityLib::BlockPosToLocalBlockPos({worldXPosB, worldYPosB, UpRegionOriginB.Z});
			FIntVector localPos1 = UChunkUtilityLib::BlockPosToLocalBlockPos({worldXPosB, worldYPosB, UpRegionOriginB.Z + 1});
			FBlockPalette* PaletteRef = &UpChunkRegion->ChunkData[UpChunkLocation].BlockPalette;
			UpperBlock = PaletteRef->GetBlockAtIndex(UChunkUtilityLib::LocalBlockPosToIndex(localPos)); // current worldPos.Z + 1
			UpperBlock1 = PaletteRef->GetBlockAtIndex(UChunkUtilityLib::LocalBlockPosToIndex(localPos1)); // current worldPos.Z + 2
			
			
			
			
			for (int32 z = FChunkRegion::RegionSizeInBlocks - 1; z >= 0; z--) // Search from top to bottom for change from solid to air block
			{
				int32 worldZPosB = regionOriginB.Z + z;
				FVector currentChunkLocation = UChunkUtilityLib::BlockPosToChunkWorldLocation({worldXPosB, worldYPosB, worldZPosB});
				FIntVector currentLocalPos = UChunkUtilityLib::BlockPosToLocalBlockPos({worldXPosB, worldYPosB, worldZPosB});
				FBlockState& currentBlock = protoRegion.ProtoChunkData[currentChunkLocation].BlockStates[UChunkUtilityLib::LocalBlockPosToIndex(currentLocalPos)];



				if(UpperBlock.IsAirBlock() && currentBlock.blockID == TERRESTRE_BLOCK_BASALT)
				{
					if (worldZPosB < UChunkUtilityLib::GetChunkManager()->TerrainShaperSettings->SeaLevel + 5)
					{
						currentBlock.blockID = TERRESTRE_BLOCK_SAND;
					}
					else
					{
						currentBlock.blockID = TERRESTRE_BLOCK_GRASS;
					}
					
						
				}
				else
				{
					switch(UpperBlock.blockID)
					{
					case TERRESTRE_BLOCK_GRASS: currentBlock.blockID = TERRESTRE_BLOCK_DIRT;
						break;
					case TERRESTRE_BLOCK_DIRT: 
						if (UpperBlock1.blockID == TERRESTRE_BLOCK_GRASS) 
						{ 
							currentBlock.blockID = TERRESTRE_BLOCK_DIRT; 
						}
						break;
					case TERRESTRE_BLOCK_SAND:
						if (UpperBlock1.blockID == TERRESTRE_BLOCK_SAND)
						{
							currentBlock.blockID = TERRESTRE_BLOCK_SAND;
						}

					}
				}
				
				UpperBlock1 = UpperBlock;
				UpperBlock = currentBlock;
			}
		}
	}
	return true;
	*/
	return false;
}