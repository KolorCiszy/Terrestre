#include "GenerateChunkMeshTask.h"

#include "DataRegistry/Public/DataRegistrySubsystem.h"
#include "Terrestre/Core/Subsystems/BlockDataSubsystem/BlockData.h"
#include "Terrestre/Core/Chunk/Misc/Directions.h"
#include "Terrestre/Core/Chunk/ChunkManager.h"
#include "Terrestre/Core/Chunk/Misc/ChunkHelper.h"
#include "Terrestre/Core/Chunk/ChunkUtilityLib.h"
#include "Terrestre/Core/Chunk/ChunkConstants.h"
#include "Terrestre/Core/Chunk/Storage/BlockPalette.h"
#include "Terrestre/Core/Subsystems/BlockDataSubsystem/BlockDataSubsystem.h"
#include "Runtime/GeometryCore/Public/DynamicMesh/DynamicMesh3.h"

FGenerateChunkMeshTask::FGenerateChunkMeshTask(AChunk* Owner)
{
	chunk = Owner;

	BlockDynamicMeshData = MakeShared<FDynamicMesh3, ESPMode::NotThreadSafe>();
	BlockDynamicMeshData->EnableVertexColors(FVector4f{});
	BlockDynamicMeshData->EnableVertexNormals(FVector3f{ 0,0,0 });
	BlockDynamicMeshData->EnableAttributes();
	BlockDynamicMeshData->Attributes()->EnablePrimaryColors();
	
	FluidDynamicMeshData = MakeShared<FDynamicMesh3, ESPMode::NotThreadSafe>();
	FluidDynamicMeshData->EnableVertexNormals(FVector3f{});
	FluidDynamicMeshData->EnableAttributes();
}

void FGenerateChunkMeshTask::GenerateBlockStateMesh()
{
	// Don't mesh if chunk contains only air
	if (chunk->IsEmpty() || !chunk->IsChunkOpacityDataReady())
	{
		return;
	}

	FChunkHelper chunkHelper;
	chunkHelper.SetSize(FChunkConstants::Volume);

	/* Cache data from neighbour chunks */
	for (EDirections direction : TEnumRange<EDirections>())
	{
		AChunk* Neighbour = chunk->GetNeighbourChunk(direction);
		int GetNeighbourTries{};
		while (!Neighbour)
		{
			FPlatformProcess::Sleep(0.05f);
			Neighbour = chunk->GetNeighbourChunk(direction);
			GetNeighbourTries++;
			if (GetNeighbourTries > 10)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Skipped waiting for neighbour chunk during mesh gen"));
				break;
			}
		}
		if (Neighbour)
		{
			int32 GetNeighbourOpacityDataTries{};
			while (!Neighbour->IsChunkOpacityDataReady())
			{
				GetNeighbourOpacityDataTries++;
				FPlatformProcess::Sleep(0.05f);
				if (GetNeighbourOpacityDataTries > 10)
				{
					//UE_LOG(LogTemp,Warning,TEXT("Skipped waiting for opacity data during mesh gen"));
					break;
				}
			}
			ChunkOpacityCache.Add(direction, Neighbour->GetChunkOpacityData());
		}
		else
		{
			FBlockOpacityData DummyData{};
			DummyData.MakeFullyOpaque();
			ChunkOpacityCache.Add(direction, MoveTemp(DummyData));
		}
	}

	chunk->GetChunkData().BlockPalette.BulkUnpack(UnpackedBlocks);

	/* Iterate over chunk's blocks */
	for (int32 x{}; x < FChunkConstants::Size; x++)
	{
		for (int32 y{}; y < FChunkConstants::Size; y++)
		{
			for (int32 z{}; z < FChunkConstants::Size; z++)
			{

				FIntVector currentPosition(x, y, z);

				int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(currentPosition) };
				/* Get Current block */
				FBlockState& block = *(UnpackedBlocks.GetData() + currentIndex);

				if (block.IsAirBlock())
				{
					continue;
				}


				int32 x1 = x + 1;
				int32 z1 = z + 1;
				int32 y1 = y + 1;
				int32 runLength = 0;
				/* Index of block that's being currently compared in the chunkHelper */
				int32 compareBlockIndex = 0;

				// Left face (Y-)
				if (!chunkHelper.visitedYN[currentIndex] && IsVisibleFace(currentPosition, EDirections::Left))
				{
					// Search upwards to determine run length
					for (int16 q = z; q < FChunkConstants::Size; q++)
					{
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ x,y,q });
						if (*(UnpackedBlocks.GetData() + compareBlockIndex) == block)
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
						CreateQuad(FVector(x, y, z + runLength) * FChunkConstants::VoxelSize,
							FVector(x1, y, z + runLength) * FChunkConstants::VoxelSize,
							FVector(x, y, z) * FChunkConstants::VoxelSize,
							FVector(x1, y, z) * FChunkConstants::VoxelSize,
							FVector::LeftVector, block);

					}
				}
				runLength = 0;
				if (!chunkHelper.visitedYP[currentIndex] && IsVisibleFace(currentPosition, EDirections::Right))
				{
					// Search upwards to determine run length
					for (int16 q = z; q < FChunkConstants::Size; q++)
					{
						// Pre-calculate the array lookup as it is used twice
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ x,y,q });
						if (*(UnpackedBlocks.GetData() + compareBlockIndex) == block)
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
						CreateQuad(FVector(x1, y1, z + runLength) * FChunkConstants::VoxelSize,
							FVector(x, y1, z + runLength) * FChunkConstants::VoxelSize,
							FVector(x1, y1, z) * FChunkConstants::VoxelSize,
							FVector(x, y1, z) * FChunkConstants::VoxelSize,
							FVector::RightVector, block);
					}
				}
				runLength = 0;
				//Forward (X+)
				if (!chunkHelper.visitedXP[currentIndex] && IsVisibleFace(currentPosition, EDirections::Forward))
				{
					// Search upwards to determine run length
					for (int16 q = z; q < FChunkConstants::Size; q++)
					{
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ x,y,q });
						if (*(UnpackedBlocks.GetData() + compareBlockIndex) == block)
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
						CreateQuad(FVector(x1, y, z + runLength) * FChunkConstants::VoxelSize,
							FVector(x1, y1, z + runLength) * FChunkConstants::VoxelSize,
							FVector(x1, y, z) * FChunkConstants::VoxelSize,
							FVector(x1, y1, z) * FChunkConstants::VoxelSize,
							FVector::ForwardVector, block);

					}
				}
				runLength = 0;
				//Backward (X-)
				if (!chunkHelper.visitedXN[currentIndex] && IsVisibleFace(currentPosition, EDirections::Backward))
				{
					// Search upwards to determine run length
					for (int16 q = z; q < FChunkConstants::Size; q++)
					{
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ x,y,q });
						if (*(UnpackedBlocks.GetData() + compareBlockIndex) == block)
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
						CreateQuad(FVector(x, y1, runLength + z) * FChunkConstants::VoxelSize,
							FVector(x, y, z + runLength) * FChunkConstants::VoxelSize,
							FVector(x, y1, z) * FChunkConstants::VoxelSize,
							FVector(x, y, z) * FChunkConstants::VoxelSize,
							FVector::BackwardVector, block);

					}
				}
				runLength = 0;
				//Up (Z+)
				if (!chunkHelper.visitedZP[currentIndex] && IsVisibleFace(currentPosition, EDirections::Up))
				{
					// Search forwards to determine run length
					for (int16 q = x; q < FChunkConstants::Size; q++)
					{
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ q,y,z });
						if (*(UnpackedBlocks.GetData() + compareBlockIndex) == block)
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
						CreateQuad(FVector(x, y, z1) * FChunkConstants::VoxelSize,
							FVector(x, y1, z1) * FChunkConstants::VoxelSize,
							FVector(x + runLength, y, z1) * FChunkConstants::VoxelSize,
							FVector(x + runLength, y1, z1) * FChunkConstants::VoxelSize,
							FVector::UpVector, block);

					}
				}
				runLength = 0;
				//Down (Z-)
				if (!chunkHelper.visitedZN[currentIndex] && IsVisibleFace(currentPosition, EDirections::Down))
				{
					// Search forwards to determine run length
					for (int16 q = x; q < FChunkConstants::Size; q++)
					{
						compareBlockIndex = UChunkUtilityLib::LocalBlockPosToIndex(FIntVector{ q,y,z });
						if (*(UnpackedBlocks.GetData() + compareBlockIndex) == block)
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
						CreateQuad(FVector(x + runLength, y, z) * FChunkConstants::VoxelSize,
							FVector(x + runLength, y1, z) * FChunkConstants::VoxelSize,
							FVector(x, y, z) * FChunkConstants::VoxelSize,
							FVector(x, y1, z) * FChunkConstants::VoxelSize,
							FVector::DownVector, block);

					}
				}
				runLength = 0;
			}
		}
	}
}
void FGenerateChunkMeshTask::GenerateWaterMesh()
{
	if (!chunk->HasAnyFluids())
	{
		return;
	}
	FluidStatesCopy = chunk->GetChunkData().FluidStates;
	
	FChunkHelper chunkHelper;
	chunkHelper.SetSize(FChunkConstants::Volume);

	for (EDirections direction : TEnumRange<EDirections>())
	{
		AChunk* Neighbour = chunk->GetNeighbourChunk(direction);
		int32 GetNeighbourTries{};
		while (!Neighbour)
		{
			FPlatformProcess::Sleep(0.05f);
			Neighbour = chunk->GetNeighbourChunk(direction);
			GetNeighbourTries++;
			if (GetNeighbourTries > 10)
			{
				UE_LOG(LogTemp, Warning, TEXT("Skipped waiting for neighbour chunk during mesh gen"));
				break;
			}
		}
		if (Neighbour)
		{
			int32 GetNeighbourOpacityDataTries{};
			while (Neighbour->GetChunkData().FluidStates.IsEmpty())
			{
				GetNeighbourOpacityDataTries++;
				FPlatformProcess::Sleep(0.05f);
				if (GetNeighbourOpacityDataTries > 10)
				{
					UE_LOG(LogTemp,Warning,TEXT("Skipped waiting for opacity data during mesh gen"));
					break;
				}
			}
			FluidStateCache.Add(direction, Neighbour->GetChunkData().FluidStates);
		}
		else
		{
			TArray<FFluidState, TInlineAllocator<FChunkConstants::Volume>> DummyData{};
			DummyData.Init(FFluidState::Empty(),DummyData.Num());
			FluidStateCache.Add(direction, MoveTemp(DummyData));
		}
		

	}
	for (int16 x{}; x < FChunkConstants::Size; x++)
	{
		for (int16 y{}; y < FChunkConstants::Size; y++)
		{
			for (int16 z{}; z < FChunkConstants::Size; z++)
			{
				FIntVector currentPosition(x, y, z);

				int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(currentPosition) };
				/* Get Current block */
				FFluidState& fluidState = *(FluidStatesCopy.GetData() + currentIndex);

				if (fluidState.fluidID != 1)
				{
					continue;
				}

				int16 x1 = x + 1;
				int16 z1 = z + 1;
				int16 y1 = y + 1;
				int16 runLength = 0;
				/* Index of block that's being currently compared in the chunkHelper */
				int32 compareFluidIndex = 0;
				/* Local fluid pos that's being currently compared in the chunkHelper */
				FIntVector compareFluidPos{};
				
				//Up (Z+)
				if (!chunkHelper.visitedZP[currentIndex] && !HasWater(currentPosition, EDirections::Up))
				{
					// Search forwards to determine run length
					for (int16 q = x; q < FChunkConstants::Size; q++)
					{
						compareFluidPos = { q,y,z };
						compareFluidIndex = UChunkUtilityLib::LocalBlockPosToIndex(compareFluidPos);

						if ((FluidStatesCopy.GetData() + compareFluidIndex)->fluidID == 1 && IsVisibleFace(compareFluidPos, EDirections::Up) && !HasWater(compareFluidPos, EDirections::Up))
						{
							chunkHelper.visitedZP[compareFluidIndex] = true;
							runLength++;
						}
						else
						{
							break;
						}
					}
					if (runLength > 0)
					{
						CreateQuad(FVector(x, y, z1) * FChunkConstants::VoxelSize,
							FVector(x, y1, z1) * FChunkConstants::VoxelSize,
							FVector(x + runLength, y, z1) * FChunkConstants::VoxelSize, // bylo x + runlength
							FVector(x + runLength, y1, z1) * FChunkConstants::VoxelSize, // bylo x + runlength
							FVector::UpVector, fluidState);

					}
				}
				/*
				runLength = 0;
				//Down (Z-)
				if (!chunkHelper.visitedZN[currentIndex] && !HasWater(currentPosition, EDirections::Down) && IsVisibleFace(currentPosition, EDirections::Down))
				{
					// Search forwards to determine run length
					for (int16 q = x; q < FChunkConstants::Size; q++)
					{
						compareFluidPos = { q,y,z };
						compareFluidIndex = UChunkUtilityLib::LocalBlockPosToIndex(compareFluidPos);

						if ((FluidStatesCopy.GetData() + compareFluidIndex)->fluidID == 1 && IsVisibleFace(compareFluidPos, EDirections::Down) && !HasWater(compareFluidPos, EDirections::Down))
						{
							chunkHelper.visitedZN[compareFluidIndex] = true;
							runLength++;
						}
						else
						{
							break;
						}
					}


					if (runLength > 0)
					{
						CreateQuad(FVector(x + runLength, y, z) * FChunkConstants::VoxelSize,
							FVector(x + runLength, y1, z) * FChunkConstants::VoxelSize,
							FVector(x, y, z) * FChunkConstants::VoxelSize,
							FVector(x, y1, z) * FChunkConstants::VoxelSize,
							FVector::DownVector, fluidState);

					}
				}
				runLength = 0;
				*/
			}
		}
	}

}
void FGenerateChunkMeshTask::DoWork()
{
	GenerateBlockStateMesh();

	GenerateWaterMesh();

	chunk->MarkMeshReady();
}
bool FGenerateChunkMeshTask::HasWater(FIntVector localPos, EDirections direction)
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
		if ((FluidStatesCopy.GetData() + currentIndex)->fluidID == 1)
		{
			return true;
		}
	}
	else
	{
		
		localPos = UChunkUtilityLib::VecModulo(localPos, FChunkConstants::Size);
		int32 currentIndex{ UChunkUtilityLib::LocalBlockPosToIndex(localPos) };
		if (auto NeighbourFluidStates = FluidStateCache.Find(direction))
		{
			if (NeighbourFluidStates->IsEmpty())
				return false;
			return NeighbourFluidStates->operator[](currentIndex).IsWater();
		}
	}
	return false;
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
	if (UChunkUtilityLib::IsValidLocalPosition(localPos)) // inside the owner chunk
	{
		int32 CompareIndex{ UChunkUtilityLib::LocalBlockPosToIndex(localPos) };
		if (UnpackedBlocks.IsEmpty() || UnpackedBlocks[CompareIndex].IsAirBlock())
		{
			return true;
		}
	}
	else // outside the owner chunk
	{
		FBlockOpacityData* NeighbourOpacity = ChunkOpacityCache.Find(direction);
		if (NeighbourOpacity)
		{
			FIntVector NeighbourLocalPos = UChunkUtilityLib::VecModulo(localPos, FChunkConstants::Size);
			int32 NeighbourLocalIndex{ UChunkUtilityLib::LocalBlockPosToIndex(NeighbourLocalPos) };
			return !ChunkOpacityCache[direction].operator[](NeighbourLocalIndex);
		}
		return true;
	}
	return false;
}
void FGenerateChunkMeshTask::CreateQuad(FVector tlv, FVector trv, FVector blv, FVector brv, FVector norm, const FBlockState& block)
{
	const FBlockData* BlockData = GEngine->GetEngineSubsystem<UBlockDataSubsystem>()->GetBlockData(block.blockID);
	if (BlockData->MeshType == EBlockMeshType::NONE)
	{
		return;
	}

	FVector3f tlv_normalized = FVector3f{ tlv } / FChunkConstants::VoxelSizeOneAxis;
	FVector3f blv_normalized = FVector3f{ blv } / FChunkConstants::VoxelSizeOneAxis;
	FVector3f brv_normalized = FVector3f{ brv } / FChunkConstants::VoxelSizeOneAxis;
	FVector3f trv_normalized = FVector3f{ trv } / FChunkConstants::VoxelSizeOneAxis;

	FVector2f uvCoords[4];
	EDirections quadDirection = GetDirectionFromNormalVector(norm);
	if (norm == FVector::DownVector || norm == FVector::UpVector)
	{
		uvCoords[0] = FVector2f{ blv_normalized.X, blv_normalized.Y };
		uvCoords[1] = FVector2f{ brv_normalized.X, brv_normalized.Y };
		uvCoords[2] = FVector2f{ tlv_normalized.X, tlv_normalized.Y };
		uvCoords[3] = FVector2f{ trv_normalized.X, trv_normalized.Y };
	}
	else if (norm == FVector::ForwardVector || norm == FVector::BackwardVector)
	{
		uvCoords[0] = FVector2f{ blv_normalized.Y, blv_normalized.Z };
		uvCoords[1] = FVector2f{ brv_normalized.Y, brv_normalized.Z };
		uvCoords[2] = FVector2f{ tlv_normalized.Y, tlv_normalized.Z };
		uvCoords[3] = FVector2f{ trv_normalized.Y, trv_normalized.Z };
	}
	else
	{
		uvCoords[0] = FVector2f{ blv_normalized.X, blv_normalized.Z };
		uvCoords[1] = FVector2f{ brv_normalized.X, brv_normalized.Z };
		uvCoords[2] = FVector2f{ tlv_normalized.X, tlv_normalized.Z };
		uvCoords[3] = FVector2f{ trv_normalized.X, trv_normalized.Z };
	}
	FVector3f textureIndex{ static_cast<float>(BlockData->GetFaceTextureIndex(quadDirection)) / 255.f,0,0 };

	auto nextIndex = BlockDynamicMeshData->VertexCount();
	int32 VertID = BlockDynamicMeshData->AppendVertex(tlv);
	
	BlockDynamicMeshData->SetVertexColor(VertID, textureIndex);
	BlockDynamicMeshData->SetVertexNormal(VertID, FVector3f{ norm });

	VertID = BlockDynamicMeshData->AppendVertex(trv);
	BlockDynamicMeshData->SetVertexColor(VertID, textureIndex);
	BlockDynamicMeshData->SetVertexNormal(VertID, FVector3f{ norm });

	VertID = BlockDynamicMeshData->AppendVertex(blv);
	BlockDynamicMeshData->SetVertexColor(VertID, textureIndex);
	BlockDynamicMeshData->SetVertexNormal(VertID, FVector3f{ norm });

	VertID = BlockDynamicMeshData->AppendVertex(brv);
	BlockDynamicMeshData->SetVertexColor(VertID, textureIndex);
	BlockDynamicMeshData->SetVertexNormal(VertID, FVector3f{ norm });

	int32 TrID0 = BlockDynamicMeshData->AppendTriangle(nextIndex + 1, nextIndex + 3, nextIndex);
	int uvIdx0 = BlockDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[1]);
	int uvIdx1 = BlockDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[3]);
	int uvIdx2 = BlockDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[0]);
	UE::Geometry::FIndex3i tri0Indices{ uvIdx0, uvIdx1, uvIdx2 };
	BlockDynamicMeshData->Attributes()->PrimaryUV()->SetTriangle(TrID0, tri0Indices);

	tri0Indices.A = BlockDynamicMeshData->Attributes()->PrimaryColors()->AppendElement( textureIndex );
	tri0Indices.B = BlockDynamicMeshData->Attributes()->PrimaryColors()->AppendElement( textureIndex );
	tri0Indices.C = BlockDynamicMeshData->Attributes()->PrimaryColors()->AppendElement( textureIndex );
	BlockDynamicMeshData->Attributes()->PrimaryColors()->SetTriangle(TrID0, tri0Indices);

	int32 TrID1 = BlockDynamicMeshData->AppendTriangle(nextIndex + 2, nextIndex, nextIndex + 3);
	int uvIdx3 = BlockDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[2]);
	int uvIdx4 = BlockDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[0]);
	int uvIdx5 = BlockDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[3]);
	UE::Geometry::FIndex3i tri1Indices{ uvIdx3, uvIdx4, uvIdx5 };
	BlockDynamicMeshData->Attributes()->PrimaryUV()->SetTriangle(TrID1, tri1Indices);

	tri1Indices.A = BlockDynamicMeshData->Attributes()->PrimaryColors()->AppendElement(textureIndex);
	tri1Indices.B = BlockDynamicMeshData->Attributes()->PrimaryColors()->AppendElement(textureIndex);
	tri1Indices.C = BlockDynamicMeshData->Attributes()->PrimaryColors()->AppendElement(textureIndex);

	BlockDynamicMeshData->Attributes()->PrimaryColors()->SetTriangle(TrID1, tri1Indices);
};

void FGenerateChunkMeshTask::CreateQuad(FVector tlv, FVector trv, FVector blv, FVector brv, FVector norm, const FFluidState& fluid)
{
	switch (fluid.fluidID)
	{
	case 1:
	{

		FVector3f tlv_normalized = FVector3f{ tlv } / FChunkConstants::VoxelSizeOneAxis;
		FVector3f blv_normalized = FVector3f{ blv } / FChunkConstants::VoxelSizeOneAxis;
		FVector3f brv_normalized = FVector3f{ brv } / FChunkConstants::VoxelSizeOneAxis;
		FVector3f trv_normalized = FVector3f{ trv } / FChunkConstants::VoxelSizeOneAxis;

		FVector2f uvCoords[4];

		if (norm == FVector::DownVector || norm == FVector::UpVector)
		{
			uvCoords[0] = FVector2f{ blv_normalized.X, blv_normalized.Y };
			uvCoords[1] = FVector2f{ brv_normalized.X, brv_normalized.Y };
			uvCoords[2] = FVector2f{ tlv_normalized.X, tlv_normalized.Y };
			uvCoords[3] = FVector2f{ trv_normalized.X, trv_normalized.Y };
		}
		else if (norm == FVector::ForwardVector || norm == FVector::BackwardVector)
		{
			uvCoords[0] = FVector2f{ blv_normalized.Y, blv_normalized.Z };
			uvCoords[1] = FVector2f{ brv_normalized.Y, brv_normalized.Z };
			uvCoords[2] = FVector2f{ tlv_normalized.Y, tlv_normalized.Z };
			uvCoords[3] = FVector2f{ trv_normalized.Y, trv_normalized.Z };
			
		}
		else
		{
			uvCoords[0] = FVector2f{ blv_normalized.X, blv_normalized.Z };
			uvCoords[1] = FVector2f{ brv_normalized.X, brv_normalized.Z };
			uvCoords[2] = FVector2f{ tlv_normalized.X, tlv_normalized.Z };
			uvCoords[3] = FVector2f{ trv_normalized.X, trv_normalized.Z };
		}

		//FColor textureIndex{ 0,0,0, static_cast<uint8>(FFluidData::GetBlockTextureIndex(block.blockID, DirectionFromNormalVector(norm))) };

		auto nextIndex = FluidDynamicMeshData->VertexCount();

		int32 VertID = FluidDynamicMeshData->AppendVertex(tlv);
		FluidDynamicMeshData->SetVertexNormal(VertID, FVector3f{ norm });

		VertID = FluidDynamicMeshData->AppendVertex(trv);
		FluidDynamicMeshData->SetVertexNormal(VertID, FVector3f{ norm });

		VertID = FluidDynamicMeshData->AppendVertex(blv);
		FluidDynamicMeshData->SetVertexNormal(VertID, FVector3f{ norm });
		
		VertID = FluidDynamicMeshData->AppendVertex(brv);
		FluidDynamicMeshData->SetVertexNormal(VertID, FVector3f{ norm });


		int32 TrID0 = FluidDynamicMeshData->AppendTriangle(nextIndex + 1, nextIndex + 3, nextIndex);
		int uvIdx0 = FluidDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[1]);
		int uvIdx1 = FluidDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[3]);
		int uvIdx2 = FluidDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[0]);
		UE::Geometry::FIndex3i tri0Indices{ uvIdx0, uvIdx1, uvIdx2 };
		FluidDynamicMeshData->Attributes()->PrimaryUV()->SetTriangle(TrID0, tri0Indices);


		int32 TrID1 = FluidDynamicMeshData->AppendTriangle(nextIndex + 2, nextIndex, nextIndex + 3);
		int uvIdx3 = FluidDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[2]);
		int uvIdx4 = FluidDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[0]);
		int uvIdx5 = FluidDynamicMeshData->Attributes()->PrimaryUV()->AppendElement(uvCoords[3]);
		UE::Geometry::FIndex3i tri1Indices{ uvIdx3, uvIdx4, uvIdx5 };
		FluidDynamicMeshData->Attributes()->PrimaryUV()->SetTriangle(TrID1, tri1Indices);




		/*
			int32 nextIndex = fluidStateMeshData->Positions.Num();

			fluidStateMeshData->Positions.Append({ tlv, trv, blv, brv });

			fluidStateMeshData->Triangles.Append({ nextIndex + 1, nextIndex + 3, nextIndex, nextIndex + 2, nextIndex, nextIndex + 3 });

			fluidStateMeshData->Normals.Append({ norm,norm,norm,norm });

			tlv /= 100;
			blv /= 100;
			brv /= 100;
			trv /= 100;
			EDirections quadDirection{};
			if (norm == FVector::DownVector || norm == FVector::UpVector)
			{
				fluidStateMeshData->UV0.Append({
				FVector2D{blv.X, blv.Y}, FVector2D{brv.X, brv.Y}, FVector2D{tlv.X, tlv.Y}, FVector2D{trv.X, trv.Y}
					});
				if (FMath::TruncToInt32(norm.Z) == 1)
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
				fluidStateMeshData->UV0.Append({
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
				fluidStateMeshData->UV0.Append({
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
		}
		*/

		break;
		}

	}
}
FGenerateChunkMeshTask::~FGenerateChunkMeshTask()
{

}