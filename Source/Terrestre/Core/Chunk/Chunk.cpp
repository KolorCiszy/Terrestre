// Fill out your copyright notice in the Description page of Project Settings.



#include "Chunk.h"
#include "ChunkUtilityLib.h"
#include "ChunkManager.h"
#include "Terrestre/Core/TerrestreNativeGameplayTags.h"
#include "Misc/Directions.h"
#include "Terrestre/Core/Character/BaseCharacter.h"
#include "Terrestre/Core/Block/BlockData.h"
#include "Async/GenerateChunkMeshTask.h"




using FMeshData = FRealtimeMeshSimpleMeshData;

// Sets default values
AChunk::AChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.TickInterval = 1.0f;

	bPendingDestroy = false;
	bBlockMeshCreated = false;
	bFluidMeshCreated = false;
	bReadyToDestroy = false;
	bMeshDirty = false;
	bMeshingTaskDone = false;
	BlockSectionConfig.DrawType = ERealtimeMeshSectionDrawType::Dynamic;
	BlockSectionConfig.MaterialSlot = 0;
	WaterSectionConfig.DrawType = ERealtimeMeshSectionDrawType::Dynamic;
	WaterSectionConfig.MaterialSlot = 1;
	WaterSectionConfig.bCastsShadow = false;
	MeshLODKey = 0;
}

void AChunk::OnGenerateMesh_Implementation()
{
	RealtimeMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>();

	RealtimeMesh->SetupMaterialSlot(0, TEXT("BLOCK STATES MATERIAL"), BlockMaterial);
	RealtimeMesh->SetupMaterialSlot(1, TEXT("WATER MATERIAL"), WaterMaterial);

	MarkMeshDirty();
}
// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();
	MeshingTask = new FAsyncTask<FGenerateChunkMeshTask>(this);
}

FBlockPalette* AChunk::GetBlockPalette() const
{
	return UChunkUtilityLib::GetChunkManager()->GetChunkBlockPalette(this->GetActorLocation());
}


// Called every frame
void AChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChunk::CreateMeshAsync()
{
	if(MeshingTask && bMeshDirty && MeshingTask->IsDone())
	{
		MeshingTask->StartBackgroundTask(UChunkUtilityLib::GetChunkManager()->ChunkMeshingTP, EQueuedWorkPriority::Highest, EQueuedWorkFlags::DoNotRunInsideBusyWait);
		bMeshingTaskDone = false;
		bMeshReady = false;
		UChunkUtilityLib::GetChunkManager()->OnRebuildChunkMeshes.AddUniqueDynamic(this, &AChunk::CreateMeshAsync);
		UChunkUtilityLib::GetChunkManager()->ActiveMeshingTasksCount++;
	}

};

void AChunk::ResetMesh()
{
	if (bBlockMeshCreated)
	{
		RealtimeMesh->RemoveSection(BlockMeshSectionKey);
		bBlockMeshCreated = false;
	}
	if (bFluidMeshCreated)
	{
		RealtimeMesh->RemoveSection(WaterMeshSectionKey);
		bFluidMeshCreated = false;
	}
	RealtimeMesh->UpdateCollision(true);
}
void AChunk::MarkMeshReady()
{
	UChunkUtilityLib::GetChunkManager()->ActiveMeshingTasksCount--;
	AsyncTask(ENamedThreads::GameThread, [&]() 
		{
			bMeshReady = true;
			UChunkUtilityLib::GetChunkManager()->OnApplyChunkMeshes.AddUniqueDynamic(this, &AChunk::ApplyMesh);
			UChunkUtilityLib::GetChunkManager()->OnRebuildChunkMeshes.RemoveDynamic(this, &AChunk::CreateMeshAsync);
		});
}

void AChunk::ApplyMesh()
{
	if (MeshingTask->IsDone())
	{
		bMeshingTaskDone = true;
		FMeshData& blockMeshData = *MeshingTask->GetTask().blockStateMeshData;
		FMeshData& fluidMeshData = *MeshingTask->GetTask().fluidStateMeshData;
		
		if (blockMeshData.Positions.Num() > 3)
		{
			if (bBlockMeshCreated)
			{
				RealtimeMesh->UpdateSectionMesh(BlockMeshSectionKey, blockMeshData);
			}
			else
			{
				BlockMeshSectionKey = RealtimeMesh->CreateMeshSection(MeshLODKey, BlockSectionConfig, blockMeshData, true);
				bBlockMeshCreated = true;
			}
		}
		else if(bBlockMeshCreated)
		{
			RealtimeMesh->RemoveSection(BlockMeshSectionKey);
			bBlockMeshCreated = false;
		}

		if (fluidMeshData.Positions.Num() > 3)
		{

			if (bFluidMeshCreated)
			{
				RealtimeMesh->UpdateSectionMesh(WaterMeshSectionKey, fluidMeshData);
			}
			else
			{
				WaterMeshSectionKey = RealtimeMesh->CreateMeshSection(MeshLODKey, WaterSectionConfig, fluidMeshData);
				bFluidMeshCreated = true;
			}
		}
		else if(bFluidMeshCreated)
		{
			RealtimeMesh->RemoveSection(WaterMeshSectionKey);
			bFluidMeshCreated = false;
		}

		MeshingTask->GetTask().ResetData();
		bMeshDirty = false;
		UChunkUtilityLib::GetChunkManager()->OnApplyChunkMeshes.RemoveDynamic(this, &AChunk::ApplyMesh);
	}
}

bool AChunk::MarkMeshDirty()
{
	if(!bMeshDirty)
	{
		bMeshDirty = true;
		UChunkUtilityLib::GetChunkManager()->OnRebuildChunkMeshes.AddUniqueDynamic(this, &AChunk::CreateMeshAsync);
		return true;
	}
	return false;
}

bool AChunk::IsEmpty() const
{
	return GetBlockPalette()->IsEmpty();
} 

AChunk* AChunk::GetNeighbourChunk(EDirections direction) const
{
	FVector location = GetActorLocation();
	switch(direction)
	{
	case EDirections::Forward: location.X += SizeScaled.X;
		break;
	case EDirections::Backward:location.X -= SizeScaled.X;
		break;
	case EDirections::Up: location.Z += SizeScaled.Z;
		break;
	case EDirections::Down:location.Z -= SizeScaled.Z;
		break;
	case EDirections::Left: location.Y -= SizeScaled.Y;
		break;
	case EDirections::Right:location.Y += SizeScaled.Y;
		break;
	default: return nullptr;
	}
	return UChunkUtilityLib::GetChunkManager()->GetChunkAtLocation(location);
}

FBlockState AChunk::GetBlockAtLocalPosition(const FIntVector localPos) const
{
	int16 index = UChunkUtilityLib::LocalBlockPosToIndex(localPos);
	return GetBlockPalette()->GetBlockAtIndex(index);
}

bool AChunk::ModifyBlockAtLocalPosition(const FIntVector localPos, const FBlockState& newBlock, bool bRequestMeshUpdate)
{
 	if(bMeshDirty)
	{
		return false;
	}
	int32 index = UChunkUtilityLib::LocalBlockPosToIndex(localPos);
	if(UChunkUtilityLib::IsValidLocalIndex(index))
	{

		GetBlockPalette()->ModifyBlockAtIndex(UChunkUtilityLib::LocalBlockPosToIndex(localPos), newBlock);
		if (bRequestMeshUpdate)
		{
			MarkMeshDirty();
		}
		else
		{
			return true;
		}
		if (localPos.X == Size - 1)
		{
			if(auto chunk = GetNeighbourChunk(EDirections::Forward))
			{
				chunk->MarkMeshDirty();
			}
		}
		if(localPos.X == 0)
		{
			if (auto chunk = GetNeighbourChunk(EDirections::Backward))
			{
				chunk->MarkMeshDirty();
			}
		}
		if (localPos.Y == Size - 1)
		{
			if (auto chunk = GetNeighbourChunk(EDirections::Right))
			{
				chunk->MarkMeshDirty();
			}
		}
		if (localPos.Y == 0)
		{
			if (auto chunk = GetNeighbourChunk(EDirections::Left))
			{
				chunk->MarkMeshDirty();
			}
		}
		if (localPos.Z == Size - 1)
		{
			if (auto chunk = GetNeighbourChunk(EDirections::Up))
			{
				chunk->MarkMeshDirty();
			}
		}
		if (localPos.Z == 0)
		{
			if (auto chunk = GetNeighbourChunk(EDirections::Down))
			{
				chunk->MarkMeshDirty();
			}
		}	
		return true;
	}
	return false;
}

void AChunk::CancelMeshingTask()
{
	if (MeshingTask)
	{
		if (MeshingTask->IsDone())
		{
			bMeshingTaskDone = true;
		}
		else
		{
			MeshingTask->Cancel();
			MeshingTask->EnsureCompletion(false, true);
			bMeshingTaskDone = true;
		}
		MeshingTask->GetTask().ResetData();
		
	}
}

void AChunk::MarkPendingDestroy() 
{
	if(bPendingDestroy)
	{
		return;
	}

	bPendingDestroy = true;
	CancelMeshingTask();
	bReadyToDestroy = true;;
};

void AChunk::EndPlay(EEndPlayReason::Type reason)
{
	MarkPendingDestroy();
	delete MeshingTask;
}

FName AChunk::OnVisibleByCharacter_Implementation(ABaseCharacter* visibleBy, const FHitResult& traceResult)
{
	FVector traceLocation = traceResult.ImpactPoint - traceResult.ImpactNormal;
	int64 blockID = GetBlockAtLocalPosition(UChunkUtilityLib::WorldLocationToLocalBlockPos(traceLocation)).blockID;
	return BlockData::GetBlockDisplayName(blockID);
}

bool AChunk::OnLeftMouseButton_Implementation(ABaseCharacter* clickedBy, const FHitResult& traceResult, int32 heldItemID)
{
	if (clickedBy->GetCharacterGameplayTags().HasTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Terrestre.Character.Player"))))
	{
		if (ModifyBlockAtLocalPosition(UChunkUtilityLib::WorldLocationToLocalBlockPos(traceResult.ImpactPoint - traceResult.ImpactNormal), FBlockState()))
		{
			return true;
		}
	}
	return false;
}

bool AChunk::SweepTestForVisibility(TArray<FHitResult>& sweepResult, FVector startLocation)
{
	
	FCollisionQueryParams params{};
	params.bIgnoreTouches = true;
	params.AddIgnoredActor(this);
	const FName TraceTag("BoxTrace");
#ifdef DEBUG
	GetWorld()->DebugDrawTraceTag = TraceTag;
#endif
	params.TraceTag = TraceTag;
	return GetWorld()->SweepMultiByChannel(sweepResult, startLocation, startLocation, FQuat::Identity,
											ECC_Visibility, FCollisionShape::MakeBox(AChunk::VoxelSize / 2), params);

}

bool AChunk::OnRightMouseButton_Implementation(ABaseCharacter* clickedBy, const FHitResult& traceResult, int32 heldItemID)
{
	FVector traceLocation = traceResult.ImpactPoint + traceResult.ImpactNormal;
	auto chunk = UChunkUtilityLib::GetChunkManager()->GetChunkAtWorldLocation(traceLocation);
	FIntVector localBlockPos = UChunkUtilityLib::WorldLocationToLocalBlockPos(traceLocation);

	FVector locationSnappedToBlockGrid = UChunkUtilityLib::SnapWorldLocationToWorldBlockLocation(traceLocation);

	TArray<FHitResult> sweepTest;
	if (SweepTestForVisibility(sweepTest, locationSnappedToBlockGrid + AChunk::VoxelSize / 2))
	{
		for (auto& hit : sweepTest)
		{
			if (!hit.GetActor()->IsA<AChunk>())
			{
				return false;
			};
		}
	}
	if(chunk == this)
	{
		return ModifyBlockAtLocalPosition(localBlockPos, FBlockState(heldItemID));
	}
	else if(chunk)
	{
		return chunk->ModifyBlockAtLocalPosition(localBlockPos, FBlockState(heldItemID));
	}
	else
	{
		return false;
	}
}
