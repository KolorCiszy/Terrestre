﻿// Copyright TriAxis Games, L.L.C. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMesh.h"
#include "RealtimeMeshConfig.h"
#include "Data/RealtimeMeshLOD.h"
#include "Data/RealtimeMeshSection.h"
#include "Data/RealtimeMeshSectionGroup.h"
#include "Interface_CollisionDataProviderCore.h"
#include "Mesh/RealtimeMeshBuilder.h"
#include "Mesh/RealtimeMeshDataStream.h"
#include "Mesh/RealtimeMeshSimpleData.h"
#include "RealtimeMeshSimple.generated.h"


#define LOCTEXT_NAMESPACE "RealtimeMesh"

class URealtimeMeshStreamSet;
class URealtimeMeshSimple;
using namespace RealtimeMesh;



enum class ERealtimeMeshSimpleSectionGroupMode : uint8
{
	TreatAsSingle,
	SeparateByGroup,
};

namespace RealtimeMesh
{
	
	class FRealtimeMeshSectionGroupSimple;

	class REALTIMEMESHCOMPONENT_API FRealtimeMeshSectionSimple : public FRealtimeMeshSection
	{
		bool bShouldCreateMeshCollision;

	public:
		FRealtimeMeshSectionSimple(const FRealtimeMeshSharedResourcesRef& InSharedResources, const FRealtimeMeshSectionKey& InKey);
		virtual ~FRealtimeMeshSectionSimple() override;

		bool HasCollision() const { return bShouldCreateMeshCollision; }
		void SetShouldCreateCollision(bool bNewShouldCreateMeshCollision);

		virtual void UpdateStreamRange(FRealtimeMeshProxyCommandBatch& Commands, const FRealtimeMeshStreamRange& InRange) override;

		virtual bool GenerateCollisionMesh(FRealtimeMeshTriMeshData& CollisionData);

		virtual bool Serialize(FArchive& Ar) override;
		virtual void Reset(FRealtimeMeshProxyCommandBatch& Commands) override;

	protected:
		virtual void HandleStreamsChanged(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshStreamKey& StreamKey, ERealtimeMeshChangeType ChangeType) const;
		virtual FBoxSphereBounds3f CalculateBounds() const override;

		void MarkCollisionDirtyIfNecessary() const;
	};

	DECLARE_DELEGATE_RetVal_OneParam(FRealtimeMeshSectionConfig, FRealtimeMeshPolyGroupConfigHandler, int32);
	
	class REALTIMEMESHCOMPONENT_API FRealtimeMeshSectionGroupSimple : public FRealtimeMeshSectionGroup
	{
	public:
	private:
		FRealtimeMeshStreamSet Streams;
		FRealtimeMeshPolyGroupConfigHandler ConfigHandler;
		
		uint8 bAutoCreateSectionsForPolygonGroups : 1;
		uint8 bIsStandalone : 1;
		
	public:
		FRealtimeMeshSectionGroupSimple(const FRealtimeMeshSharedResourcesRef& InSharedResources, const FRealtimeMeshSectionGroupKey& InKey)
			: FRealtimeMeshSectionGroup(InSharedResources, InKey)
			, bAutoCreateSectionsForPolygonGroups(true)
			, bIsStandalone(false)
		{
		}

		void FlagStandalone() { bIsStandalone = true; }
		bool IsStandalone() const { return bIsStandalone; }

		FRealtimeMeshSectionPtr GetStandaloneSection() const;

		FRealtimeMeshStreamRange GetStreamRange() const;
		
		const FRealtimeMeshStream* GetStream(FRealtimeMeshStreamKey StreamKey) const;

		void SetPolyGroupSectionHandler(const FRealtimeMeshPolyGroupConfigHandler& NewHandler);
		void ClearPolyGroupSectionHandler();

		TFuture<ERealtimeMeshProxyUpdateStatus> EditMeshData(TFunctionRef<TSet<FRealtimeMeshStreamKey>(FRealtimeMeshStreamSet&)> EditFunc);

		virtual void CreateOrUpdateStream(FRealtimeMeshProxyCommandBatch& Commands, FRealtimeMeshStream&& Stream) override;
		virtual void RemoveStream(FRealtimeMeshProxyCommandBatch& Commands, const FRealtimeMeshStreamKey& StreamKey) override;
		
		TFuture<ERealtimeMeshProxyUpdateStatus> UpdateFromSimpleMesh(const FRealtimeMeshSimpleMeshData& MeshData);
		void UpdateFromSimpleMesh(FRealtimeMeshProxyCommandBatch& Commands, const FRealtimeMeshSimpleMeshData& MeshData);

		virtual void InitializeProxy(FRealtimeMeshProxyCommandBatch& Commands) override;

		virtual void Reset(FRealtimeMeshProxyCommandBatch& Commands) override;
		virtual bool Serialize(FArchive& Ar) override;
		
		virtual bool GenerateCollisionMesh(FRealtimeMeshTriMeshData& CollisionData);
	protected:

		virtual void UpdatePolyGroupSections(FRealtimeMeshProxyCommandBatch& Commands, bool bUpdateDepthOnly);
		virtual FRealtimeMeshSectionConfig DefaultPolyGroupSectionHandler(int32 PolyGroupIndex) const;
	};

	class REALTIMEMESHCOMPONENT_API FRealtimeMeshLODSimple : public FRealtimeMeshLODData
	{
	public:
		FRealtimeMeshLODSimple(const FRealtimeMeshSharedResourcesRef& InSharedResources, const FRealtimeMeshLODKey& InKey)
			: FRealtimeMeshLODData(InSharedResources, InKey)
		{
		}

		virtual bool GenerateCollisionMesh(FRealtimeMeshTriMeshData& CollisionData);
	};

	DECLARE_MULTICAST_DELEGATE(FRealtimeMeshSimpleCollisionDataChangedEvent);

	class REALTIMEMESHCOMPONENT_API FRealtimeMeshSharedResourcesSimple : public FRealtimeMeshSharedResources
	{
		FRealtimeMeshSimpleCollisionDataChangedEvent CollisionDataChangedEvent;

	public:
		FRealtimeMeshSimpleCollisionDataChangedEvent& OnCollisionDataChanged() { return CollisionDataChangedEvent; }
		void BroadcastCollisionDataChanged() const { CollisionDataChangedEvent.Broadcast(); }


		virtual FRealtimeMeshSectionRef CreateSection(const FRealtimeMeshSectionKey& InKey) const override
		{
			return MakeShared<FRealtimeMeshSectionSimple>(ConstCastSharedRef<FRealtimeMeshSharedResources>(this->AsShared()), InKey);
		}

		virtual FRealtimeMeshSectionGroupRef CreateSectionGroup(const FRealtimeMeshSectionGroupKey& InKey) const override
		{
			return MakeShared<FRealtimeMeshSectionGroupSimple>(ConstCastSharedRef<FRealtimeMeshSharedResources>(this->AsShared()), InKey);
		}

		virtual FRealtimeMeshLODDataRef CreateLOD(const FRealtimeMeshLODKey& InKey) const override
		{
			return MakeShared<FRealtimeMeshLODSimple>(ConstCastSharedRef<FRealtimeMeshSharedResources>(this->AsShared()), InKey);
		}

		virtual FRealtimeMeshRef CreateRealtimeMesh() const override;
		virtual FRealtimeMeshSharedResourcesRef CreateSharedResources() const override { return MakeShared<FRealtimeMeshSharedResourcesSimple>(); }
	};

	class REALTIMEMESHCOMPONENT_API FRealtimeMeshSimple : public FRealtimeMesh
	{
	protected:
		FRealtimeMeshCollisionConfiguration CollisionConfig;
		FRealtimeMeshSimpleGeometry SimpleGeometry;
		mutable TSharedPtr<TPromise<ERealtimeMeshCollisionUpdateResult>> PendingCollisionPromise;

	public:
		FRealtimeMeshSimple(const FRealtimeMeshSharedResourcesRef& InSharedResources)
			: FRealtimeMesh(InSharedResources)
		{
			SharedResources->As<FRealtimeMeshSharedResourcesSimple>().OnCollisionDataChanged().AddRaw(this, &FRealtimeMeshSimple::MarkCollisionDirtyNoCallback);
		}

		virtual ~FRealtimeMeshSimple() override
		{
			SharedResources->As<FRealtimeMeshSharedResourcesSimple>().OnCollisionDataChanged().RemoveAll(this);
		}

		FRealtimeMeshCollisionConfiguration GetCollisionConfig() const;
		TFuture<ERealtimeMeshCollisionUpdateResult> SetCollisionConfig(const FRealtimeMeshCollisionConfiguration& InCollisionConfig);
		FRealtimeMeshSimpleGeometry GetSimpleGeometry() const;
		TFuture<ERealtimeMeshCollisionUpdateResult> SetSimpleGeometry(const FRealtimeMeshSimpleGeometry& InSimpleGeometry);

		virtual bool GenerateCollisionMesh(FRealtimeMeshTriMeshData& CollisionData);

		virtual void Reset(FRealtimeMeshProxyCommandBatch& Commands, bool bRemoveRenderProxy) override;

		virtual bool Serialize(FArchive& Ar) override;

	protected:
		void MarkForEndOfFrameUpdate() const;
		TFuture<ERealtimeMeshCollisionUpdateResult> MarkCollisionDirty() const;
		void MarkCollisionDirtyNoCallback() const;

		virtual void ProcessEndOfFrameUpdates() override;

		friend class URealtimeMeshSimple;
	};
}


DECLARE_DYNAMIC_DELEGATE_OneParam(FRealtimeMeshSimpleCompletionCallback, ERealtimeMeshProxyUpdateStatus, ProxyUpdateResult);

DECLARE_DYNAMIC_DELEGATE_OneParam(FRealtimeMeshSimpleCollisionCompletionCallback, ERealtimeMeshCollisionUpdateResult, CollisionResult);


UCLASS(Blueprintable)
class REALTIMEMESHCOMPONENT_API URealtimeMeshSimple : public URealtimeMesh
{
	GENERATED_UCLASS_BODY()
protected:

public:
	TSharedRef<RealtimeMesh::FRealtimeMeshSimple> GetMeshData() const { return StaticCastSharedRef<RealtimeMesh::FRealtimeMeshSimple>(GetMesh()); }

	TFuture<ERealtimeMeshProxyUpdateStatus> CreateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey);

	TFuture<ERealtimeMeshProxyUpdateStatus> CreateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, FRealtimeMeshStreamSet&& MeshData);
	TFuture<ERealtimeMeshProxyUpdateStatus> CreateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshStreamSet& MeshData);
	
	TFuture<ERealtimeMeshProxyUpdateStatus> UpdateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, FRealtimeMeshStreamSet&& MeshData);
	TFuture<ERealtimeMeshProxyUpdateStatus> UpdateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshStreamSet& MeshData);
	
	// DEPRECATE
	TFuture<ERealtimeMeshProxyUpdateStatus> CreateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSimpleMeshData& MeshData);
	TFuture<ERealtimeMeshProxyUpdateStatus> UpdateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSimpleMeshData& MeshData);

	
	TFuture<ERealtimeMeshProxyUpdateStatus> CreateSection(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSectionConfig& Config,
															const FRealtimeMeshStreamRange& StreamRange, bool bShouldCreateCollision = false);

	TFuture<ERealtimeMeshProxyUpdateStatus> UpdateSectionConfig(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSectionConfig& Config);
	TFuture<ERealtimeMeshProxyUpdateStatus> UpdateSectionRange(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshStreamRange& StreamRange);

	
	TFuture<ERealtimeMeshProxyUpdateStatus> EditMeshInPlace(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const TFunctionRef<TSet<FRealtimeMeshStreamKey>(FRealtimeMeshStreamSet&)>&);
	


	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSectionGroup", meta= (AutoCreateRefTerm = "CompletionCallback"))
	void CreateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, URealtimeMeshStreamSet* MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	UE_DEPRECATED(5.0, "FRealtimeMeshSimpleMeshData is deprecated, use URealtimeMeshStreamSet instead and its helper functions")
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSectionGroupFromSimpleMesh", meta= (AutoCreateRefTerm = "CompletionCallback"))
	void CreateSectionGroupFromSimple(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSimpleMeshData& MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSectionGroupUnique", meta= (AutoCreateRefTerm = "CompletionCallback"))
	FRealtimeMeshSectionGroupKey CreateSectionGroupUnique(const FRealtimeMeshLODKey& LODKey, URealtimeMeshStreamSet* MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	UE_DEPRECATED(5.0, "FRealtimeMeshSimpleMeshData is deprecated, use URealtimeMeshStreamSet instead and its helper functions")
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSectionGroupUniqueFromSimpleMesh", meta= (AutoCreateRefTerm = "CompletionCallback"))
	FRealtimeMeshSectionGroupKey CreateSectionGroupUniqueFromSimple(const FRealtimeMeshLODKey& LODKey,const FRealtimeMeshSimpleMeshData& MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="UpdateSectionGroup", meta= (AutoCreateRefTerm = "CompletionCallback"))
	void UpdateSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, URealtimeMeshStreamSet* MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	
	UE_DEPRECATED(5.0, "FRealtimeMeshSimpleMeshData is deprecated, use URealtimeMeshStreamSet instead and its helper functions")
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="UpdateSectionGroupFromSimpleMesh", meta= (AutoCreateRefTerm = "CompletionCallback"))
	void UpdateSectionGroupFromSimple(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSimpleMeshData& MeshData, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);



	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSection", meta = (AutoCreateRefTerm = "Config, StreamRange, CompletionCallback"))
	void CreateSection(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSectionConfig& Config,
								 const FRealtimeMeshStreamRange& StreamRange, bool bShouldCreateCollision, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	UE_DEPRECATED(all, "Use CreateSection instead")
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="CreateSectionUnique", meta = (AutoCreateRefTerm = "Config, StreamRange, CompletionCallback"))
	FRealtimeMeshSectionKey CreateSectionUnique(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSectionConfig& Config,
														  const FRealtimeMeshStreamRange& StreamRange, bool bShouldCreateCollision,
														  const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	


	//void EditMeshInPlace(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const TFunctionRef<TSet<FRealtimeMeshStreamKey>(FRealtimeMeshStreamSet&)>&);



	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="UpdateSectionConfig", meta = (AutoCreateRefTerm = "SectionKey"))
	void UpdateSectionConfig(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSectionConfig& Config,
									   const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);




	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="RemoveSection", meta = (AutoCreateRefTerm = "SectionKey"))
	void RemoveSection(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="RemoveSectionGroup", meta = (AutoCreateRefTerm = "SectionGroupKey"))
	void RemoveSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);


	


	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", meta = (AutoCreateRefTerm = "SectionKey"))
	FRealtimeMeshSectionConfig GetSectionConfig(const FRealtimeMeshSectionKey& SectionKey) const;

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", meta = (AutoCreateRefTerm = "SectionKey"))
	bool IsSectionVisible(const FRealtimeMeshSectionKey& SectionKey) const;

	TFuture<ERealtimeMeshProxyUpdateStatus> SetSectionVisibility(const FRealtimeMeshSectionKey& SectionKey, bool bIsVisible);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="SetSectionVisibility", meta = (AutoCreateRefTerm = "SectionKey"))
	void SetSectionVisibility(const FRealtimeMeshSectionKey& SectionKey, bool bIsVisible, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", meta = (AutoCreateRefTerm = "SectionKey"))
	bool IsSectionCastingShadow(const FRealtimeMeshSectionKey& SectionKey) const;

	TFuture<ERealtimeMeshProxyUpdateStatus> SetSectionCastShadow(const FRealtimeMeshSectionKey& SectionKey, bool bCastShadow);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="SetSectionCastShadow", meta = (AutoCreateRefTerm = "SectionKey"))
	void SetSectionCastShadow(const FRealtimeMeshSectionKey& SectionKey, bool bCastShadow, const FRealtimeMeshSimpleCompletionCallback& CompletionCallback);
	
	TFuture<ERealtimeMeshProxyUpdateStatus> RemoveSection(const FRealtimeMeshSectionKey& SectionKey);

	TFuture<ERealtimeMeshProxyUpdateStatus> RemoveSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey);


	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh")
	FRealtimeMeshCollisionConfiguration GetCollisionConfig() const;

	TFuture<ERealtimeMeshCollisionUpdateResult> SetCollisionConfig(const FRealtimeMeshCollisionConfiguration& InCollisionConfig);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="SetCollisionConfig")
	void SetCollisionConfig(const FRealtimeMeshCollisionConfiguration& InCollisionConfig, const FRealtimeMeshSimpleCollisionCompletionCallback& CompletionCallback);
	
	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh")
	FRealtimeMeshSimpleGeometry GetSimpleGeometry() const;

	TFuture<ERealtimeMeshCollisionUpdateResult> SetSimpleGeometry(const FRealtimeMeshSimpleGeometry& InSimpleGeometry);

	UFUNCTION(BlueprintCallable, Category = "Components|RealtimeMesh", DisplayName="SetSimpleGeometry")
	void SetSimpleGeometry(const FRealtimeMeshSimpleGeometry& InSimpleGeometry, const FRealtimeMeshSimpleCollisionCompletionCallback& CompletionCallback);
	
	virtual void Reset(bool bCreateNewMeshData) override;
	
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
};

#undef LOCTEXT_NAMESPACE
