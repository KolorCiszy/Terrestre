#include "GenerateChunkMeshTask.h"

#include "DataRegistry/Public/DataRegistrySubsystem.h"
#include "Terrestre/Core/Block/BlockData.h"
#include "Terrestre/Core/Chunk/Misc/Directions.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Terrestre/Core/Chunk/Misc/ChunkHelper.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/Storage/BlockState.h"
#include "Terrestre/Core/Chunk/Storage/FluidState.h"
#include "Terrestre/Core/Chunk/Storage/BlockPalette.h"

void FGenerateChunkMeshTask::ResetData()
{
	forwardChunkDataValid = false;
	backwardChunkDataValid = false;
	rightChunkDataValid = false;
	leftChunkDataValid = false;
	upChunkDataValid = false;
	downChunkDataValid = false;
	ClearMeshData(*blockStateMeshData);
	ClearMeshData(*fluidStateMeshData);
	uncompressedBlocks.Empty();
	uncompressedBlocksF.Empty();
	uncompressedBlocksB.Empty();
	uncompressedBlocksL.Empty();
	uncompressedBlocksR.Empty();
	uncompressedBlocksU.Empty();
	uncompressedBlocksD.Empty();
}
void FGenerateChunkMeshTask::GenerateBlockStateMesh()
{
	if (chunk->IsEmpty())
	{
		return;
	}

	FChunkHelper chunkHelper;
	chunkHelper.SetSize(AChunk::Volume);

	/* Cache data from neighbour chunks */

	FVector forwardChunkLocation = chunk->GetActorLocation() + FVector{ AChunk::SizeScaled.X, 0, 0 };
	FVector backwardChunkLocation = chunk->GetActorLocation() - FVector{ AChunk::SizeScaled.X, 0 ,0 };
	FVector rightChunkLocation = chunk->GetActorLocation() + FVector{ 0 , AChunk::SizeScaled.Y, 0 };
	FVector leftChunkLocation = chunk->GetActorLocation() - FVector{ 0, AChunk::SizeScaled.Y, 0 };
	FVector upChunkLocation = chunk->GetActorLocation() + FVector{ 0, 0, AChunk::SizeScaled.Z };
	FVector downChunkLocation = chunk->GetActorLocation() - FVector{ 0,0, AChunk::SizeScaled.Z };


	if (UChunkUtilityLib::GetChunkManager()->BulkUnpackChunkBlocks(forwardChunkLocation, uncompressedBlocksF))
	{
		forwardChunkDataValid = true;
	};
	if (UChunkUtilityLib::GetChunkManager()->BulkUnpackChunkBlocks(backwardChunkLocation, uncompressedBlocksB))
	{
		backwardChunkDataValid = true;
	};
	if (UChunkUtilityLib::GetChunkManager()->BulkUnpackChunkBlocks(rightChunkLocation, uncompressedBlocksR))
	{
		rightChunkDataValid = true;
	};
	if (UChunkUtilityLib::GetChunkManager()->BulkUnpackChunkBlocks(leftChunkLocation, uncompressedBlocksL))
	{
		leftChunkDataValid = true;
	};
	if (UChunkUtilityLib::GetChunkManager()->BulkUnpackChunkBlocks(upChunkLocation, uncompressedBlocksU))
	{
		upChunkDataValid = true;
	};
	if (UChunkUtilityLib::GetChunkManager()->BulkUnpackChunkBlocks(downChunkLocation, uncompressedBlocksD))
	{
		downChunkDataValid = true;
	};

	UChunkUtilityLib::GetChunkManager()->BulkUnpackChunkBlocks(chunk->GetActorLocation(), uncompressedBlocks);

	/* Iterate over chunk's blocks */
	for (int16 x{}; x < AChunk::Size; x++)
	{
		for (int16 y{}; y < AChunk::Size; y++)
		{
			for (int16 z{}; z < AChunk::Size; z++)
			{

				FIntVector currentPosition(x, y, z);
				int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(currentPosition) };
				/* Get Current block */
				FBlockState& block = *(uncompressedBlocks.GetData() + currentIndex);

				if (block.IsAirBlock())
				{
					continue;
				}


				int16 x1 = x + 1;
				int16 z1 = z + 1;
				int16 y1 = y + 1;
				int16 runLength = 0;
				/* Index of block that's being currently compared in the chunkHelper */
				int32 compareBlockIndex = 0;

				// Left face (Y-)
				if (!chunkHelper.visitedYN[currentIndex] && IsVisibleFace(currentPosition, EDirections::Left))
				{
					// Search upwards to determine run length
					for (int16 q = z; q < AChunk::Size; q++)
					{
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ x,y,q });
						if (*(uncompressedBlocks.GetData() + compareBlockIndex) == block)
						{
							chunkHelper.visitedYN[compareBlockIndex] = true;
							runLength++;
						}
						else
						{
							break;
						}
					}
					if (runLength > 0)
					{
						// Create a quad and write it directly to the buffer
						CreateQuad(FVector(x, y, z + runLength) * AChunk::VoxelSize,
							FVector(x1, y, z + runLength) * AChunk::VoxelSize,
							FVector(x, y, z) * AChunk::VoxelSize,
							FVector(x1, y, z) * AChunk::VoxelSize,
							FVector::LeftVector, block);

					}
				}
				runLength = 0;
				if (!chunkHelper.visitedYP[currentIndex] && IsVisibleFace(currentPosition, EDirections::Right))
				{
					// Search upwards to determine run length
					for (int16 q = z; q < AChunk::Size; q++)
					{
						// Pre-calculate the array lookup as it is used twice
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ x,y,q });
						if (*(uncompressedBlocks.GetData() + compareBlockIndex) == block)
						{
							chunkHelper.visitedYP[compareBlockIndex] = true;
							runLength++;
						}
						else
						{
							break;
						}
					}
					if (runLength > 0)
					{
						CreateQuad(FVector(x1, y1, z + runLength) * AChunk::VoxelSize,
							FVector(x, y1, z + runLength) * AChunk::VoxelSize,
							FVector(x1, y1, z) * AChunk::VoxelSize,
							FVector(x, y1, z) * AChunk::VoxelSize,
							FVector::RightVector, block);
					}
				}
				runLength = 0;
				//Forward (X+)
				if (!chunkHelper.visitedXP[currentIndex] && IsVisibleFace(currentPosition, EDirections::Forward))
				{
					// Search upwards to determine run length
					for (int16 q = z; q < AChunk::Size; q++)
					{
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ x,y,q });
						if (*(uncompressedBlocks.GetData() + compareBlockIndex) == block)
						{
							chunkHelper.visitedXP[compareBlockIndex] = true;
							runLength++;
						}
						else
						{
							break;
						}
					}
					if (runLength > 0)
					{
						CreateQuad(FVector(x1, y, z + runLength) * AChunk::VoxelSize,
							FVector(x1, y1, z + runLength) * AChunk::VoxelSize,
							FVector(x1, y, z) * AChunk::VoxelSize,
							FVector(x1, y1, z) * AChunk::VoxelSize,
							FVector::ForwardVector, block);

					}
				}
				runLength = 0;
				//Backward (X-)
				if (!chunkHelper.visitedXN[currentIndex] && IsVisibleFace(currentPosition, EDirections::Backward))
				{
					// Search upwards to determine run length
					for (int16 q = z; q < AChunk::Size; q++)
					{
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ x,y,q });
						if (*(uncompressedBlocks.GetData() + compareBlockIndex) == block)
						{
							chunkHelper.visitedXN[compareBlockIndex] = true;
							runLength++;
						}
						else
						{
							break;
						}
					}
					if (runLength > 0)
					{
						CreateQuad(FVector(x, y1, runLength + z) * AChunk::VoxelSize,
							FVector(x, y, z + runLength) * AChunk::VoxelSize,
							FVector(x, y1, z) * AChunk::VoxelSize,
							FVector(x, y, z) * AChunk::VoxelSize,
							FVector::BackwardVector, block);

					}
				}
				runLength = 0;
				//Up (Z+)
				if (!chunkHelper.visitedZP[currentIndex] && IsVisibleFace(currentPosition, EDirections::Up))
				{
					// Search forwards to determine run length
					for (int16 q = x; q < AChunk::Size; q++)
					{
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ q,y,z });
						if (*(uncompressedBlocks.GetData() + compareBlockIndex) == block)
						{
							chunkHelper.visitedZP[compareBlockIndex] = true;
							runLength++;
						}
						else
						{
							break;
						}
					}
					if (runLength > 0)
					{
						CreateQuad(FVector(x, y, z1) * AChunk::VoxelSize,
							FVector(x, y1, z1) * AChunk::VoxelSize,
							FVector(x + runLength, y, z1) * AChunk::VoxelSize, // bylo x + runlength
							FVector(x + runLength, y1, z1) * AChunk::VoxelSize, // bylo x + runlength
							FVector::UpVector, block);

					}
				}
				runLength = 0;
				//Down (Z-)
				if (!chunkHelper.visitedZN[currentIndex] && IsVisibleFace(currentPosition, EDirections::Down))
				{
					// Search forwards to determine run length
					for (int16 q = x; q < AChunk::Size; q++)
					{
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ q,y,z });
						if (*(uncompressedBlocks.GetData() + compareBlockIndex) == block)
						{
							chunkHelper.visitedZN[compareBlockIndex] = true;
							runLength++;
						}
						else
						{
							break;
						}
					}


					if (runLength > 0)
					{
						CreateQuad(FVector(x + runLength, y, z) * AChunk::VoxelSize,
							FVector(x + runLength, y1, z) * AChunk::VoxelSize,
							FVector(x, y, z) * AChunk::VoxelSize,
							FVector(x, y1, z) * AChunk::VoxelSize,
							FVector::DownVector, block);

					}
				}
				runLength = 0;
			}
		}
	}
}
void FGenerateChunkMeshTask::GenerateFluidStateMesh()
{
	TArray<FFluidState, TInlineAllocator<AChunk::Volume>>* fluidStates = chunk->GetFluidStates();
	for (int16 x{}; x < AChunk::Size; x++)
	{
		for (int16 y{}; y < AChunk::Size; y++)
		{
			for (int16 z{}; z < AChunk::Size; z++)
			{

			}
		}
	}
	
}
void FGenerateChunkMeshTask::DoWork()
{
	GenerateBlockStateMesh();

	GenerateFluidStateMesh();

	chunk->MarkMeshReady();	
}

bool FGenerateChunkMeshTask::IsVisibleFace(FIntVector localPos, EDirections direction)
{
	switch (direction)
	{
	case EDirections::Forward: localPos.X++;
		break;
	case EDirections::Backward: localPos.X--;
		break;
	case EDirections::Right: localPos.Y++;
		break;
	case EDirections::Left: localPos.Y--;
		break;
	case EDirections::Up: localPos.Z++;
		break;
	case EDirections::Down: localPos.Z--;
		break;
	default: return true;
	}
	if (UChunkUtilityLib::IsValidLocalPosition(localPos))
	{
		int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(localPos) };
		if ((uncompressedBlocks.GetData() + currentIndex)->IsAirBlock())
		{
			return true;
		}
	}
	else
	{
		if (localPos.X >= AChunk::Size && forwardChunkDataValid)
		{
			localPos.X = 0;
			int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(localPos) };
			if ((uncompressedBlocksF.GetData() + currentIndex)->IsAirBlock())
			{
				return true;
			}	
		}
		else if(localPos.X < 0 && backwardChunkDataValid)
		{
			localPos.X = AChunk::Size - 1;
			int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(localPos) };
			if ((uncompressedBlocksB.GetData() + currentIndex)->IsAirBlock())
			{
				return true;
			}
		}
		else if (localPos.Y >= AChunk::Size && rightChunkDataValid)
		{
			localPos.Y = 0;
			int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(localPos) };
			if ((uncompressedBlocksR.GetData() + currentIndex)->IsAirBlock())
			{
				return true;
			}	
			
		}
		else if (localPos.Y < 0 && leftChunkDataValid)
		{
			localPos.Y = AChunk::Size - 1;
			int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(localPos) };
			if ((uncompressedBlocksL.GetData() + currentIndex)->IsAirBlock())
			{
				return true;
			}
		}
		else if (localPos.Z >= AChunk::Size && upChunkDataValid)
		{
			localPos.Z = 0;
			int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(localPos) };
			if ((uncompressedBlocksU.GetData() + currentIndex)->IsAirBlock())
			{
				return true;
			}
		}
		else if (localPos.Z < 0 && downChunkDataValid)
		{
			localPos.Z = AChunk::Size - 1;
			int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(localPos) };
			if ((uncompressedBlocksD.GetData() + currentIndex)->IsAirBlock())
			{
				return true;
			}
		}
	}
	return false;
}
void FGenerateChunkMeshTask::CreateQuad(FVector tlv, FVector trv, FVector blv, FVector brv, FVector norm, const FBlockState& block)
{
	
	if(BlockData::GetBlockMeshType(block.blockID) == EBlockMeshType::NONE)
	{
		return;
	}
	int32 nextIndex = blockStateMeshData->Positions.Num();
	blockStateMeshData->Positions.Append({tlv, trv, blv, brv});
	
	blockStateMeshData->Triangles.Append({nextIndex + 1, nextIndex + 3, nextIndex, nextIndex + 2, nextIndex, nextIndex + 3});

	blockStateMeshData->Normals.Append({norm,norm,norm,norm});

	tlv /= 100;
	blv /= 100;
	brv /= 100;
	trv /= 100;
	EDirections quadDirection{};
	if (norm == FVector::DownVector || norm == FVector::UpVector)
	{
		blockStateMeshData->UV0.Append({
		FVector2D{blv.X, blv.Y}, FVector2D{brv.X, brv.Y}, FVector2D{tlv.X, tlv.Y}, FVector2D{trv.X, trv.Y}
			});
		if(FMath::TruncToInt32(norm.Z) == 1)
		{
			quadDirection = EDirections::Up;
		}
		else
		{
			quadDirection = EDirections::Down;
		}

	}
	else if (norm == FVector::ForwardVector || norm == FVector::BackwardVector)
	{
		blockStateMeshData->UV0.Append({
		FVector2D{blv.Y, blv.Z}, FVector2D{brv.Y, brv.Z}, FVector2D{tlv.Y, tlv.Z}, FVector2D{trv.Y, trv.Z}
			});
		if (FMath::TruncToInt32(norm.X) == 1)
		{
			quadDirection = EDirections::Forward;
		}
		else
		{
			quadDirection = EDirections::Backward;
		}
	}	
	else
	{
		blockStateMeshData->UV0.Append({
		FVector2D{blv.X, blv.Z}, FVector2D{brv.X, brv.Z}, FVector2D{tlv.X, tlv.Z}, FVector2D{trv.X, trv.Z}
			});
		if (FMath::TruncToInt32(norm.Y) == 1)
		{
			quadDirection = EDirections::Right;
		}
		else
		{
			quadDirection = EDirections::Left;
		}
	}
	FColor textureIndex{ 0,0,0, static_cast<uint8>(BlockData::GetBlockTextureIndex(block.blockID, quadDirection))};
	blockStateMeshData->Colors.Append({ textureIndex, textureIndex, textureIndex, textureIndex });
	
}
;
