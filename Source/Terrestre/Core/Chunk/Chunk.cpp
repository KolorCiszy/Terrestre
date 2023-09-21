// Fill out your copyright notice in the Description page of Project Settings.



#include "Chunk.h"
#include "ChunkUtilityLib.h"
#include "ChunkManager.h"
#include "Misc/Directions.h"
#include "ProceduralMeshComponent.h"
#include "Terrestre/Core/Player/PlayerCharacter.h"
#include "Terrestre/Core/Block/BlockData.h"
#include "Async/GenerateChunkMeshTask.h"
#include "RealtimeMeshComponent.h"




using FMeshData = FRealtimeMeshSimpleMeshData;

// Sets default values
AChunk::AChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.TickInterval = 1.0f;

	bPendingDestroy = false;
	bMeshCreated = false;
	bReadyToDestroy = false;
	bMeshDirty = false;
	bMeshingTaskDone = false;

	
#ifdef USE_PROCEDURAL_MESH
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	SetRootComponent(ProceduralMesh);
	ProceduralMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
#else

	RealtimeMeshComponent = CreateDefaultSubobject<URealtimeMeshComponent>(TEXT("RealtimeMeshComponent"));
	RealtimeMeshComponent->SetMobility(EComponentMobility::Movable);
	RealtimeMeshComponent->SetGenerateOverlapEvents(false);
	RealtimeMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	SetRootComponent(RealtimeMeshComponent);

	//RealtimeMeshComponent->CollisionType = ECollisionTraceFlag::CTF_UseDefault;
#endif
	
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();
	
	
#ifdef USE_PROCEDURAL_MESH
	ProceduralMesh->SetMaterial(0, primaryMaterial);
#else
	realtimeMesh = RealtimeMeshComponent->InitializeRealtimeMesh<URealtimeMeshSimple>();
	realtimeMesh->SetupMaterialSlot(0, TEXT("PRIMARY MATERIAL"), primaryMaterial);

#endif
	meshingTask = MakeUnique<FAsyncTask<FGenerateChunkMeshTask>>(this);
	MarkMeshDirty();
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
	if(meshingTask && bMeshDirty && meshingTask->IsDone())
	{
		
		meshingTask->StartBackgroundTask(UChunkUtilityLib::GetChunkManager()->ChunkMeshingTP, EQueuedWorkPriority::Highest, EQueuedWorkFlags::DoNotRunInsideBusyWait);
		bMeshingTaskDone = false;
		bMeshReady = false;
		UChunkUtilityLib::GetChunkManager()->OnRebuildChunkMeshes.AddUniqueDynamic(this, &AChunk::CreateMeshAsync);
		UChunkUtilityLib::GetChunkManager()->ActiveMeshingTasksCount++;
	}

};

void AChunk::ClearMeshSection(int32 sectionNum)
{
#ifdef USE_PROCEDURAL_MESH
	ProceduralMesh->ClearMeshSection(sectionNum);
#else
	realtimeMesh->RemoveSection(meshSectionKey);
#endif
	bMeshCreated = false;
}
void AChunk::MarkMeshReady()
{
	UChunkUtilityLib::GetChunkManager()->ActiveMeshingTasksCount--;
	AsyncTask(ENamedThreads::GameThread, [&]() 
		{
			
			bMeshReady = true;
			UChunkUtilityLib::GetChunkManager()->OnApplyChunkMeshes.AddUniqueDynamic(this, &AChunk::ApplyMesh);
		});
}
void AChunk::ApplyMesh()
{
	if (meshingTask->IsDone()) 
	{
		bMeshingTaskDone = true;
		FMeshData& meshData = *meshingTask->GetTask().meshData;
		if (meshData.Positions.Num() > 3)
		{
#ifdef USE_PROCEDURAL_MESH
			ProceduralMesh->CreateMeshSection(0,
				meshData.Positions,
				meshData.Triangles,
				meshData.Normals,
				meshData.UV0,
				meshData.Colors,
				TArray<FProcMeshTangent>{}, true);
			bMeshCreated.store(true);
#else
		
			if (!meshSectionKey.IsValid())
			{
				FRealtimeMeshSectionConfig config{ ERealtimeMeshSectionDrawType::Dynamic, 0 };
				meshSectionKey = realtimeMesh->CreateMeshSection(0, config, meshData, true);
			}
			else
			{
				realtimeMesh->UpdateSectionMesh(meshSectionKey, meshData);
			}
			

#endif

			
		}
		meshingTask->GetTask().ResetData();
		bMeshDirty = false;
		UChunkUtilityLib::GetChunkManager()->OnApplyChunkMeshes.RemoveDynamic(this, &AChunk::ApplyMesh);
		//return true;
	}
	//return false;

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

AChunk* AChunk::GetNeighbourChunk(EDirections direction)
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
		MarkMeshDirty();
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
	if (meshingTask)
	{
		if (meshingTask->IsDone())
		{
			bMeshingTaskDone = true;
		}
		else
		{
			meshingTask->Cancel();
			meshingTask->EnsureCompletion(false, true);
			bMeshingTaskDone = true;
		}
		meshingTask->GetTask().ResetData();
		
	}
}
void AChunk::MarkPendingDestroy() 
{
	if(bPendingDestroy.load())
	{
		return;
	}

	bPendingDestroy.store(1);
	CancelMeshingTask();
	bReadyToDestroy.store(1);
};
void AChunk::EndPlay(EEndPlayReason::Type reason)
{
	MarkPendingDestroy();
}
bool AChunk::IsInsideBounds(FVector inWorldLocation)
{
	FBox Box{};
	Box.BuildAABB(GetActorLocation(), SizeScaled);
	return Box.IsInside(inWorldLocation);
}
FName AChunk::OnVisibleByCharacter_Implementation(ACharacter* visibleBy, const FHitResult& traceResult)
{
	//GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Cyan, TEXT("lalala"), true);
	FVector traceLocation = traceResult.ImpactPoint - traceResult.ImpactNormal;
	int64 blockID = GetBlockAtLocalPosition(UChunkUtilityLib::WorldLocationToLocalBlockPos(traceLocation)).blockID;
	return BlockData::GetBlockDisplayName(blockID);
}
bool AChunk::OnLeftMouseButton_Implementation(ACharacter* clickedBy, const FHitResult& traceResult, int64 heldItemID)
{
	if(TObjectPtr<APlayerCharacter> player = Cast<APlayerCharacter>(clickedBy))
	{
		
		ModifyBlockAtLocalPosition(UChunkUtilityLib::WorldLocationToLocalBlockPos(traceResult.ImpactPoint - traceResult.ImpactNormal), FBlockState());
		
		return true;
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

bool AChunk::OnRightMouseButton_Implementation(ACharacter* clickedBy, const FHitResult& traceResult, int64 heldItemID)
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
