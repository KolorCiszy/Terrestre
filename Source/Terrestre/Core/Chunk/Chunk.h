// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "Storage/BlockPalette.h"
#include "Terrestre/Core/Interfaces/InteractableActor.h"
#include "RealtimeMeshSimple.h"
#include "RealtimeMeshActor.h"
#include "Chunk.generated.h"


#define USE_PROCEDURAL_MESH

struct FRealtimeMeshSimpleMeshData;
using FMeshData = FRealtimeMeshSimpleMeshData;
class UBoxComponent;
class UProceduralMeshComponent;
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
	inline bool IsEmpty() const;
	UFUNCTION(BlueprintCallable, Category = "Chunk")
	bool ModifyBlockAtLocalPosition(const FIntVector localPos, const FBlockState& newBlock, bool bRequestMeshUpdate = true);
	
	/* Checks if give location is within chunk bounds*/
	UFUNCTION(BlueprintPure, CAtegory = "Chunk")
	bool IsInsideBounds(FVector inWorldLocation);

	bool inline MarkMeshDirty();

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

	std::atomic<bool> bMeshCreated;

	std::atomic<bool> bMeshReady;

	bool bMeshDirty;

	

	//TObjectPtr<URealtimeMeshSimple> RealtimeMesh;
	
	//FCriticalSection mutex;
	
	/* Get Chunk's block data stored in a form of indexed palette, the data is owned by chunk manager */
	FBlockPalette* GetBlockPalette() const;

	UFUNCTION()
	void CreateMeshAsync();

	void inline MarkMeshReady();

	void inline CancelMeshingTask();

	FAsyncTask<FGenerateChunkMeshTask>* meshingTask;


	//FRealtimeMeshSectionKey meshSectionKey;

	UPROPERTY(Category = "Procedural Mesh", VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Mesh,Rendering,Physics,Components|StaticMesh", AllowPrivateAccess = "true"))
	TObjectPtr<UProceduralMeshComponent> ProceduralMesh;
	/*
	UPROPERTY(Category = "Bounds Box", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> BoundsBox;
	*/
	bool SweepTestForVisibility(TArray<FHitResult>& sweepResult, FVector startLocation);

	AChunk* GetNeighbourChunk(EDirections direction);
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* primaryMaterial;

	// * Always executes on the game thread
	bool ApplyMesh();

	void inline ClearMeshSection(int32 sectionNum);
};
