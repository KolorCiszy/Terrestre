// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <atomic>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Terrestre/Core/Interfaces/InteractableActor.h"
#include "ChunkGenStage.h"
#include "ChunkProviderInterface.h"
#include "BlockOpacityData.h"
#include "HAL/CriticalSection.h"
#include "Runtime/GeometryFramework/Public/Components/DynamicMeshComponent.h"
#include "Async/GenerateChunkMeshTask.h"
#include "Chunk.generated.h"


struct FFluidState;

class FGenerateChunkMeshTask;
class FBlockPalette;
class UDynamicMeshComponent;

DECLARE_DELEGATE(FChunkDataReadyDelegate)

UCLASS(Blueprintable)
class TERRESTRE_API AChunk : public AActor, public IInteractableActor, public IChunkProviderInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChunk();
	friend void FGenerateChunkMeshTask::DoWork();
	//friend class FGenerateChunkMeshTask;
	friend class AChunkManager;

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	FBlockState GetBlockAtLocalPosition(const FIntVector localPos) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	FFluidState GetFluidAtLocalPosition(const FIntVector localPos) const;
	
	UFUNCTION(BlueprintPure, Category = "Chunk")
	bool IsEmpty() const;

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	bool ModifyBlockAtLocalPosition(const FIntVector localPos, const FBlockState& newBlock, bool bRequestMeshUpdate = true);

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	AChunk* GetNeighbourChunk(EDirections direction) const;


	UFUNCTION(BlueprintPure, Category = "Chunk")
	bool IsReadyToDestroy() const;

	UFUNCTION(BlueprintPure, Category = "Chunk")
	EChunkGenStage GetGenStage() const
	{
		return ChunkData.GenStage;
	}
	UFUNCTION(BlueprintPure, Category = "Chunk")
	bool IsBorderChunk() const
	{
		return bBorderChunk;
	}

	FChunkData GetChunkData() const
	{
		FRWScopeLock ReadLock(ChunkDataLock, FRWScopeLockType::SLT_ReadOnly);
		return ChunkData;
	}
	FBlockOpacityData GetChunkOpacityData() const
	{
		FRWScopeLock ReadLock(ChunkDataLock, FRWScopeLockType::SLT_ReadOnly);
		return OpacityData;
	}
	bool IsChunkOpacityDataReady() const
	{
		UE::TReadScopeLock ReadLock(ChunkDataLock);
		return OpacityData.IsReady;	
	}
	bool HasAnyFluids() const
	{
		FRWScopeLock ReadLock(ChunkDataLock, FRWScopeLockType::SLT_ReadOnly);
		return ChunkData.FluidStates.Contains(FFluidState{1,100});
	}
	bool IsTerrainShapeDataReady() const
	{
		return bTerrainShapeDataReady;
	}

	FChunkDataReadyDelegate ChunkDataReady;
protected:
	
	virtual void PostActorCreated() override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type reason) override;

	inline void MarkPendingDestroy();

	virtual FInteractionResult OnVisibleByCharacter_Implementation(ABaseCharacter* visibleBy, const FHitResult& traceResult) override;

	virtual FInteractionResult OnLeftMouseButton_Implementation(ABaseCharacter* clickedBy, const FHitResult& traceResult) override;
	
	virtual FInteractionResult OnRightMouseButton_Implementation(ABaseCharacter* clickedBy, const FHitResult& traceResult) override;

	void MarkMeshDirty();

	bool SweepTestForVisibility(TArray<FHitResult>& sweepResult, FVector startLocation);
private:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void QueryDataProvider();

	void DataReady();

	void UpdateOpacityData();

	void ChangeBorderChunkStatus(bool bNewBorderChunkStatus);

	FChunkData ChunkData;

	FBlockOpacityData OpacityData;

	TFuture<FChunkData> ProcessedChunkData;

	bool bPendingDestroy : 1;

	bool bReadyToDestroy : 1;

	bool bBlockMeshCreated : 1;

	bool bFluidMeshCreated : 1;
		
	bool bMeshReady : 1;

	bool bMeshingTaskDone : 1;

	bool bBorderChunk : 1;

	bool bMeshPendingUpdate : 1;

	std::atomic<bool> bTerrainShapeDataReady;
// *** Mesh Stuff ***//

	FAsyncTask<FGenerateChunkMeshTask>* MeshingTask;

	mutable FRWLock ChunkDataLock;

	void CreateMeshAsync();

	void MarkMeshReady();

	FDelegateHandle MeshDelegateHandle;

	// * Always executes on the game thread
	void ApplyMesh();

	void ResetMesh();

	void CancelMeshingTask();
	

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BlockMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* WaterMaterial;
	
	UPROPERTY()
	UDynamicMeshComponent* BlockMeshDynamic;

	UPROPERTY()
	UDynamicMeshComponent* FluidMeshDynamic;
};
