// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Terrestre/Core/Interfaces/InteractableActor.h"
#include "RealtimeMeshActor.h"
#include "Chunk.generated.h"



struct FRealtimeMeshSimpleMeshData;
struct FFluidState;

using FMeshData = FRealtimeMeshSimpleMeshData;

class URealtimeMeshComponent;
class URealtimeMeshSimple;
class FGenerateChunkMeshTask;
class FBlockPalette;


UCLASS(Blueprintable)
class TERRESTRE_API AChunk : public ARealtimeMeshActor, public IInteractableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChunk();

	friend class FGenerateChunkMeshTask;
	friend class AChunkManager;

	//* STATIC MEMBERS *//
	
	//* Size in regular blocks
	static inline constexpr uint8 Size = 16u;
	static inline constexpr uint32 SizeSquared = Size * Size;
	static inline constexpr uint32 Volume = SizeSquared * Size;
	
	//* Size of regular block
	static inline FVector VoxelSize { 100.0, 100.0, 100.0 };
	static inline FIntVector VoxelIntSize{ VoxelSize };
	//* Chunk size scaled by unreal units 
	static inline FVector SizeScaled = VoxelSize * Size;
	
	static inline FRealtimeMeshSectionConfig BlockSectionConfig; 
	static inline FRealtimeMeshSectionConfig WaterSectionConfig;
	
	static inline FRealtimeMeshLODKey MeshLODKey;

	//******************//

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	FBlockState GetBlockAtLocalPosition(const FIntVector localPos) const;

	UFUNCTION(BlueprintPure, Category = "Chunk")
	bool IsEmpty() const;

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	bool ModifyBlockAtLocalPosition(const FIntVector localPos, const FBlockState& newBlock, bool bRequestMeshUpdate = true);

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	AChunk* GetNeighbourChunk(EDirections direction) const;

	virtual void OnGenerateMesh_Implementation() override;
protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type reason) override;

	inline void MarkPendingDestroy();

	virtual FName OnVisibleByCharacter_Implementation(ABaseCharacter* visibleBy, const FHitResult& traceResult) override;

	virtual bool OnLeftMouseButton_Implementation(ABaseCharacter* clickedBy, const FHitResult& traceResult, int32 heldItemID) override;
	
	virtual bool OnRightMouseButton_Implementation(ABaseCharacter* clickedBy, const FHitResult& traceResult, int32 heldItemID) override;

	bool MarkMeshDirty();

	bool SweepTestForVisibility(TArray<FHitResult>& sweepResult, FVector startLocation);
private:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool bPendingDestroy;

	bool bReadyToDestroy;

	bool bBlockMeshCreated;

	bool bFluidMeshCreated;
		
	bool bMeshReady;

	bool bMeshDirty;

	bool bMeshingTaskDone;

	FAsyncTask<FGenerateChunkMeshTask>* MeshingTask;

	/* Get Chunk's block data stored in a form of indexed palette, the data is owned by chunk manager */
	FBlockPalette* GetBlockPalette() const;

	/* Get Chunk's fluid data stored in an array, the data is owned by chunk manager */
	//TArray<FFluidState, TInlineAllocator<AChunk::Volume>>* GetFluidStates() const; // TODO

	UFUNCTION()
	void CreateMeshAsync();

	void MarkMeshReady();

	UFUNCTION()		// * Always executes on the game thread
	void ApplyMesh();

	void ResetMesh();

	void CancelMeshingTask();
	
	UPROPERTY()
	FRealtimeMeshSectionKey BlockMeshSectionKey;

	UPROPERTY()
	FRealtimeMeshSectionKey WaterMeshSectionKey;

	UPROPERTY()
	TObjectPtr<URealtimeMeshSimple> RealtimeMesh;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BlockMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* WaterMaterial;
	

};
