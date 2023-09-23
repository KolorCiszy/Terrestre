// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Terrestre/Core/Interfaces/InteractableActor.h"
#include "RealtimeMeshSimple.h"
#include "Chunk.generated.h"


//#define USE_PROCEDURAL_MESH

struct FRealtimeMeshSimpleMeshData;
struct FFluidState;
using FMeshData = FRealtimeMeshSimpleMeshData;
class UBoxComponent;
class UProceduralMeshComponent;
class URealtimeMeshComponent;
class URealtimeMeshSimple;
class FGenerateChunkMeshTask;
class FBlockPalette;

enum class EDirections;



UCLASS(Blueprintable)
class TERRESTRE_API AChunk : public AActor, public IInteractableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChunk();

	friend class FGenerateChunkMeshTask;
	friend class AChunkManager;

	//* Size in regular blocks
	static inline constexpr uint8 Size = 16u;
	static inline constexpr uint32 SizeSquared = Size * Size;
	static inline constexpr uint32 Volume = SizeSquared * Size;
	
	//* Size of regular block
	static inline FVector VoxelSize{ 100.0, 100.0, 100.0 };
	static inline FIntVector VoxelIntSize{ VoxelSize };
	//* Chunk size scaled by unreal units 
	static inline FVector SizeScaled = VoxelSize * Size;
	


	UFUNCTION(BlueprintCallable, Category = "Chunk")
	FBlockState GetBlockAtLocalPosition(const FIntVector localPos) const;
	UFUNCTION(BlueprintCallable, Category = "Chunk")
	bool IsEmpty() const;
	UFUNCTION(BlueprintCallable, Category = "Chunk")
	bool ModifyBlockAtLocalPosition(const FIntVector localPos, const FBlockState& newBlock, bool bRequestMeshUpdate = true);


	
	/* Checks if give location is within chunk bounds*/
	UFUNCTION(BlueprintPure, CAtegory = "Chunk")
	bool IsInsideBounds(FVector inWorldLocation);

	bool MarkMeshDirty();
	
protected:
	
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type reason) override;

	inline void MarkPendingDestroy();

	virtual FName OnVisibleByCharacter_Implementation(ACharacter* visibleBy, const FHitResult& traceResult) override;

	virtual bool OnLeftMouseButton_Implementation(ACharacter* clickedBy, const FHitResult& traceResult, int64 heldItemID) override;
	
	virtual bool OnRightMouseButton_Implementation(ACharacter* clickedBy, const FHitResult& traceResult, int64 heldItemID) override;
private:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	std::atomic<bool> bPendingDestroy;

	std::atomic<bool> bReadyToDestroy;

	std::atomic<bool> bMeshingTaskDone;

	std::atomic<bool> bBlockMeshCreated;

	std::atomic<bool> bMeshReady;

	bool bMeshDirty;

	bool bFluidMeshCreated;
	
	/* Get Chunk's block data stored in a form of indexed palette, the data is owned by chunk manager */
	FBlockPalette* GetBlockPalette() const;

	/* Get Chunk's fluid data stored in an array, the data is owned by chunk manager */
	//TArray<FFluidState, TInlineAllocator<AChunk::Volume>>* GetFluidStates() const;

	UFUNCTION()
	void CreateMeshAsync();

	void MarkMeshReady();

	void CancelMeshingTask();

	TUniquePtr<FAsyncTask<FGenerateChunkMeshTask>> meshingTask;

	UPROPERTY(Category = "Realtime Mesh", VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Mesh,Rendering,Physics,Components|StaticMesh", AllowPrivateAccess = "true"))
	TObjectPtr<URealtimeMeshComponent> RealtimeMeshComponent;

	FRealtimeMeshSectionKey blockMeshSectionKey;

	FRealtimeMeshSectionKey fluidMeshSectionKey;
	
	UPROPERTY()
	URealtimeMeshSimple* realtimeMesh;

	bool SweepTestForVisibility(TArray<FHitResult>& sweepResult, FVector startLocation);

	AChunk* GetNeighbourChunk(EDirections direction) const;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* BlockMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* WaterMaterial;
	
	UFUNCTION()		// * Always executes on the game thread
	void ApplyMesh();
	
	void ClearMeshSection(int32 sectionNum);
};
