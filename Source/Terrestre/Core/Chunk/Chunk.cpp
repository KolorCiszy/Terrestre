// Fill out your copyright notice in the Description page of Project Settings.



#include "Chunk.h"
#include "ChunkUtilityLib.h"
#include "ChunkManager.h"
#include "ChunkConstants.h"
#include "Terrestre/Core/TerrestreNativeGameplayTags.h"
#include "Misc/Directions.h"
#include "Terrestre/Core/Character/BaseCharacter.h"
#include "Async/GenerateChunkMeshTask.h"
#include "Terrestre/Core/Components/InventoryComponent.h"
#include "Terrestre/Core/GameMode/TerrestrePlayerState.h"
#include <Terrestre/Core/Gamemode/TerrestreGameInstance.h>
#include <Terrestre/Core/Subsystems/WorldGenSubsystem/WorldGenSubsystem.h>
#include <Terrestre/Core/Subsystems/BlockDataSubsystem/BlockDataSubsystem.h>
#include "Terrestre/Core/Subsystems/BlockDataSubsystem/BlockData.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Terrestre/Core/Subsystems/ItemDataSubsystem/ItemEntityFactory.h"


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
	bMeshingTaskDone = false;
	
	BlockMeshDynamic = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("BlockMeshDynamic"));
	RootComponent = BlockMeshDynamic;

	BlockMeshDynamic->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	FluidMeshDynamic = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("FluidMeshDynamic"));

	FluidMeshDynamic->SetupAttachment(RootComponent);
	FluidMeshDynamic->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FluidMeshDynamic->CastShadow = false;
	
	BlockMeshDynamic->EnableComplexAsSimpleCollision();
	BlockMeshDynamic->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);
	BlockMeshDynamic->SetDeferredCollisionUpdatesEnabled(true);
	BlockMeshDynamic->SetTangentsType(EDynamicMeshComponentTangentsMode::AutoCalculated);
	
}
void AChunk::PostActorCreated()
{
	Super::PostActorCreated();
	FluidMeshDynamic->SetMaterial(0, WaterMaterial);
	BlockMeshDynamic->SetMaterial(0, BlockMaterial);
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();
	
	ChunkDataReady.BindUObject(this, &AChunk::DataReady);

	QueryDataProvider();
	
}

void AChunk::QueryDataProvider()
{
	GetOwner<AChunkManager>()->ActiveChunkDataProviderTasksCount++;
	ProcessedChunkData = GetOwner<AChunkManager>()->ProvideChunkData(this);
}
void AChunk::DataReady()
{
	GetOwner<AChunkManager>()->ActiveChunkDataProviderTasksCount--;
	auto ScopeLock = FRWScopeLock(ChunkDataLock,SLT_Write);
	UpdateOpacityData();
	
	ChunkData = ProcessedChunkData.Get();
	ProcessedChunkData.Reset();
	bTerrainShapeDataReady = true;
	Async(EAsyncExecution::TaskGraphMainThread, [this]()
		{
		
		
			
			if (bBorderChunk)
			{
				if (ChunkData.GenStage != EChunkGenStage::TerrainShape)
				{
					QueryDataProvider();
				}
			}
			else
			{
				if (ChunkData.GenStage == EChunkGenStage::Full)
				{
					CreateMeshAsync();
					for (EDirections Dir : TEnumRange<EDirections>())
					{
						if (auto Chunk = GetNeighbourChunk(Dir))
						{
							Chunk->MarkMeshDirty();
						}
					}
				}
				else 
				{
					QueryDataProvider();
				}
			}
		});
	
}
void AChunk::UpdateOpacityData()
{
	auto DataSubsystem = GEngine->GetEngineSubsystem<UBlockDataSubsystem>();
	for (int32 i{}; i < FChunkConstants::Volume; i++)
	{
		FBlockState BlockState = ChunkData.BlockPalette.GetBlockAtIndex(i);
		bool bIsOpaque = DataSubsystem->GetBlockData(BlockState.blockID)->bIsOpaque;
		OpacityData.SetBlockOpacityAtIndex(i, bIsOpaque);
	}
	OpacityData.IsReady = true;
}
void AChunk::ChangeBorderChunkStatus(bool bNewBorderChunkStatus)
{
	if (bBorderChunk == bNewBorderChunkStatus)
	{
		return; // No change needed
	}
	if (bBorderChunk && GetGenStage() == EChunkGenStage::TerrainShape)
	{
		// If we are changing from border chunk to non-border chunk, we need to query data provider
		QueryDataProvider();
	}
	else
	{
		MarkMeshDirty();// If we are changing from non-border chunk to border chunk, we do nothing for now
	}
	bBorderChunk = bNewBorderChunkStatus;

}
// Called every frame
void AChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChunk::CreateMeshAsync()
{
	ensure(IsInGameThread());
	if (MeshingTask)
	{
		return; // Task already exists, no need to create a new one
	}

	if (auto ChunkManager = GetOwner<AChunkManager>())
	{
		MeshingTask = new FAsyncTask<FGenerateChunkMeshTask>(this);
		MeshingTask->StartBackgroundTask(ChunkManager->ChunkMeshingTP, EQueuedWorkPriority::Highest);
		bMeshingTaskDone = false;
		bMeshReady = false;

		++ChunkManager->ActiveMeshingTasksCount;
	}
}

void AChunk::ResetMesh()
{
	
	if (bBlockMeshCreated)
	{
		BlockMeshDynamic->GetDynamicMesh()->Reset();
		bBlockMeshCreated = false;
	}
	if (bFluidMeshCreated)
	{
		FluidMeshDynamic->GetDynamicMesh()->Reset();
		bFluidMeshCreated = false;
	}
}
void AChunk::MarkMeshReady()
{
	Async(EAsyncExecution::TaskGraphMainThread, [this]()
		{
			ApplyMesh();
		});
}

void AChunk::ApplyMesh()
{
	ensure(IsInGameThread());
	if (MeshingTask)
	{
		MeshingTask->EnsureCompletion();
	}
	if (MeshingTask->IsDone())
	{
		bMeshingTaskDone = true;
		FDynamicMesh3& blockMeshDataDynamic = (*MeshingTask->GetTask().BlockDynamicMeshData);
		FDynamicMesh3& fluidMeshDataDynamic = (*MeshingTask->GetTask().FluidDynamicMeshData);

		BlockMeshDynamic->EditMesh([&](FDynamicMesh3& EditMesh)
			{
				EditMesh.Copy(blockMeshDataDynamic);

			}, EDynamicMeshComponentRenderUpdateMode::FullUpdate);
		//BlockMeshDynamic->NotifyMeshUpdated();
		BlockMeshDynamic->UpdateCollision(false);
		bBlockMeshCreated = true;

		FluidMeshDynamic->EditMesh([&](FDynamicMesh3& EditMesh)
			{
				EditMesh.Copy(fluidMeshDataDynamic);
			}, EDynamicMeshComponentRenderUpdateMode::FullUpdate);


		bMeshPendingUpdate = false;
		
		delete MeshingTask;
		MeshingTask = nullptr;
		if (auto ChunkManager = GetOwner<AChunkManager>())
		{
			--ChunkManager->ActiveMeshingTasksCount;
		}
	}
}

void AChunk::MarkMeshDirty()
{
	if(!bMeshPendingUpdate && !bBorderChunk)
	{
		bMeshPendingUpdate = true;
		CreateMeshAsync();
	}
}

bool AChunk::IsReadyToDestroy() const
{
	return bReadyToDestroy;
}

bool AChunk::IsEmpty() const
{
	if (bPendingDestroy)
	{
		return true;
	}
	return ChunkData.BlockPalette.IsEmpty();
} 

AChunk* AChunk::GetNeighbourChunk(EDirections direction) const
{
	FVector location = GetActorLocation();
	switch(direction)
	{
	case EDirections::Forward: location.X += FChunkConstants::SizeScaled.X;
		break;
	case EDirections::Backward:location.X -= FChunkConstants::SizeScaled.X;
		break;
	case EDirections::Up: location.Z += FChunkConstants::SizeScaled.Z;
		break;
	case EDirections::Down:location.Z -= FChunkConstants::SizeScaled.Z;
		break;
	case EDirections::Left: location.Y -= FChunkConstants::SizeScaled.Y;
		break;
	case EDirections::Right:location.Y += FChunkConstants::SizeScaled.Y;
		break;
	default: return nullptr;
	}
	auto Manager = GetOwner<AChunkManager>();
	AChunk* Chunk{};
	if (Manager)
	{
		Chunk = Manager->GetChunkAtLocation(location);
	}
	return Chunk;
}

FBlockState AChunk::GetBlockAtLocalPosition(const FIntVector localPos) const
{
	int16 index = UChunkUtilityLib::LocalBlockPosToIndex(localPos);
	return ChunkData.BlockPalette.GetBlockAtIndex(index);
}
FFluidState AChunk::GetFluidAtLocalPosition(const FIntVector localPos) const
{
	int16 index = UChunkUtilityLib::LocalBlockPosToIndex(localPos);
	return ChunkData.FluidStates.IsValidIndex(index) ? ChunkData.FluidStates[index] : FFluidState{};
}

bool AChunk::ModifyBlockAtLocalPosition(const FIntVector localPos, const FBlockState& newBlock, bool bRequestMeshUpdate)
{
 	if(bMeshPendingUpdate)
	{
		return false;
	}
	int32 index = UChunkUtilityLib::LocalBlockPosToIndex(localPos);
	if(UChunkUtilityLib::IsValidLocalIndex(index))
	{
		ChunkData.BlockPalette.ModifyBlockAtIndex(UChunkUtilityLib::LocalBlockPosToIndex(localPos), newBlock);
		auto DataSubsystem = GEngine->GetEngineSubsystem<UBlockDataSubsystem>();
		
		OpacityData.SetBlockOpacityAtIndex(index, DataSubsystem->GetBlockData(newBlock.blockID)->bIsOpaque);
		if (bRequestMeshUpdate)
		{
			MarkMeshDirty();
		}
		else
		{
			return true;
		}
		if (localPos.X == FChunkConstants::Size - 1)
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
		if (localPos.Y == FChunkConstants::Size - 1)
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
		if (localPos.Z == FChunkConstants::Size - 1)
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
		{MeshingTask->TryAbandonTask();
			MeshingTask->Cancel();
			MeshingTask->WaitCompletionWithTimeout(0.5f);
			bMeshingTaskDone = true;
		}
		delete MeshingTask;
		MeshingTask = nullptr;
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
	bReadyToDestroy = true;
};

void AChunk::EndPlay(EEndPlayReason::Type reason)
{
	Super::EndPlay(reason);
	MarkPendingDestroy();
	delete MeshingTask;
}

FInteractionResult AChunk::OnVisibleByCharacter_Implementation(ABaseCharacter* visibleBy, const FHitResult& traceResult)
{
	FInteractionResult result{};
	result.InteractedWith = this;
	result.bSuccess = true;
	FVector traceLocation = traceResult.ImpactPoint - traceResult.ImpactNormal;
	int64 blockID = GetBlockAtLocalPosition(UChunkUtilityLib::WorldLocationToLocalBlockPos(traceLocation)).blockID;
	return result;
}

FInteractionResult AChunk::OnLeftMouseButton_Implementation(ABaseCharacter* clickedBy, const FHitResult& traceResult)
{
	FInteractionResult result{};
	result.InteractedWith = this;
	if (clickedBy->GetCharacterGameplayTags().HasTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Terrestre.Character.Player"))))
	{
		auto LocalBlockPos = UChunkUtilityLib::WorldLocationToLocalBlockPos(traceResult.ImpactPoint - traceResult.ImpactNormal);

		FBlockState HitBlock = ChunkData.BlockPalette.GetBlockAtLocalPos(LocalBlockPos);
		if (ModifyBlockAtLocalPosition(LocalBlockPos, FBlockState()))
		{
			result.bSuccess = true;
			result.bShouldRefreshInventoryUI = true;
			
			auto const BlockDataSubsystem = GEngine->GetEngineSubsystem<UBlockDataSubsystem>();
			auto const BlockData = BlockDataSubsystem->GetBlockData(HitBlock.blockID);
			
			if (clickedBy->GetPlayerState<ATerrestrePlayerState>()->GetPlayerGameMode() == EPlayerGameMode::Survival && BlockData)
			{
				FTransform SpawnTransform{traceResult.Location};
				
				UItemEntityFactory::SpawnEntityItem(this, SpawnTransform, 1, BlockData->DroppedItemID);
			};
		}
	}
	return result;
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
											ECC_Visibility, FCollisionShape::MakeBox(FChunkConstants::VoxelSize / 2), params);

}

FInteractionResult AChunk::OnRightMouseButton_Implementation(ABaseCharacter* clickedBy, const FHitResult& traceResult)
{
	FInteractionResult result{};
	result.InteractedWith = this;

	TWeakObjectPtr<UItemBase> heldItem{};
	if (clickedBy->GetCharacterGameplayTags().HasTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Terrestre.Character.Player"))))
	{
		UInventoryComponent* InvComp = clickedBy->GetPlayerState<ATerrestrePlayerState>()->GetComponentByClass<UInventoryComponent>();
		
		heldItem = InvComp->GetHeldItem();
	}
	
	if (!heldItem.Get())
	{	
		result.bSuccess = false;
		return result;
	}
	TArray<FHitResult> sweepTest;

	FVector traceLocation = traceResult.ImpactPoint + traceResult.ImpactNormal;
	auto chunk = GetOwner<AChunkManager>()->GetChunkAtWorldLocation(traceLocation);
	FIntVector localBlockPos = UChunkUtilityLib::WorldLocationToLocalBlockPos(traceLocation);

	FVector locationSnappedToBlockGrid = UChunkUtilityLib::SnapWorldLocationToWorldBlockLocation(traceLocation);

	if (SweepTestForVisibility(sweepTest, locationSnappedToBlockGrid + FChunkConstants::VoxelSize / 2))
	{
		for (auto& hit : sweepTest)
		{
			if (!hit.GetActor()->IsA<AChunk>())
			{
				return result; // failed to place block
			};
		}
	}
	if(chunk == this)
	{
		result.bSuccess = ModifyBlockAtLocalPosition(localBlockPos, FBlockState::MakeBlockState(heldItem.Get()));
	}
	else if(chunk)
	{
		result.bSuccess = chunk->ModifyBlockAtLocalPosition(localBlockPos, FBlockState::MakeBlockState(heldItem.Get()));
	}
	if (result.bSuccess)
	{
		result.bShouldRefreshInventoryUI = true;
		heldItem->SetItemCount(heldItem->GetItemCount() - 1);
	}
	return result;
}
