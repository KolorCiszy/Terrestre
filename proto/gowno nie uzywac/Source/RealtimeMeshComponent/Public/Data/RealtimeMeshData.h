﻿// Copyright TriAxis Games, L.L.C. All Rights Reserved.

#pragma once

#include "RealtimeMeshCore.h"
#include "RealtimeMeshConfig.h"
#include "RealtimeMeshCollision.h"
#include "Data/RealtimeMeshShared.h"
#include "Async/Async.h"

struct FTriMeshCollisionData;

namespace RealtimeMesh
{
	struct FRealtimeMeshProxyCommandBatch;
	struct FRealtimeMeshUpdateContext;


	class REALTIMEMESHCOMPONENT_API FRealtimeMesh : public TSharedFromThis<FRealtimeMesh, ESPMode::ThreadSafe>
	{
	protected:
		const FRealtimeMeshSharedResourcesRef SharedResources;
		mutable FRealtimeMeshProxyPtr RenderProxy;
		TFixedLODArray<FRealtimeMeshLODDataRef> LODs;
		FRealtimeMeshConfig Config;
		FRealtimeMeshBounds Bounds;

		//FRealtimeMeshCollisionConfiguration CollisionConfig;
		//FRealtimeMeshSimpleGeometry SimpleGeometry;

		// Name of the mesh, given by the parent URealtimeMesh for debug messaging.
		/*FName MeshName;*/

		//bool bIsCollisionDirty;

		/*mutable FRealtimeMeshGuard Guard;*/

		/*mutable FRWLock RenderDataLock;
		mutable FRWLock CollisionLock;
		mutable FRWLock BoundsLock;*/

		/*FName TypeName;*/
	public:
		FRealtimeMesh(const FRealtimeMeshSharedResourcesRef& InSharedResources);
		virtual ~FRealtimeMesh() = default;

		const FRealtimeMeshSharedResourcesRef& GetSharedResources() const { return SharedResources; }

		/*FName GetMeshName() const { return MeshName; }*/
		/*void SetMeshName(FName InMeshName) { MeshName = InMeshName; }*/

		int32 GetNumLODs() const;

		virtual FBoxSphereBounds3f GetLocalBounds() const;
		//bool IsCollisionDirty() const { return bIsCollisionDirty; }
		//void ClearCollisionDirtyFlag() { bIsCollisionDirty = false; }


		/*FRealtimeMeshCollisionConfiguration GetCollisionConfig() const;
		void SetCollisionConfig(const FRealtimeMeshCollisionConfiguration& InCollisionConfig);
		FRealtimeMeshSimpleGeometry GetSimpleGeometry() const;
		void SetSimpleGeometry(const FRealtimeMeshSimpleGeometry& InSimpleGeometry);*/

		FRealtimeMeshLODDataPtr GetLOD(FRealtimeMeshLODKey LODKey) const;

		template <typename LODType>
		TSharedPtr<LODType, ESPMode::ThreadSafe> GetLODAs(FRealtimeMeshLODKey LODKey) const
		{
			return StaticCastSharedPtr<LODType>(GetLOD(LODKey));
		}

		FRealtimeMeshSectionGroupPtr GetSectionGroup(FRealtimeMeshSectionGroupKey SectionGroupKey) const;
		template <typename SectionGroupType>
		TSharedPtr<SectionGroupType> GetSectionGroupAs(const FRealtimeMeshSectionGroupKey& SectionGroupKey) const
		{
			return StaticCastSharedPtr<SectionGroupType>(GetSectionGroup(SectionGroupKey));
		}
		FRealtimeMeshSectionPtr GetSection(FRealtimeMeshSectionKey SectionKey) const;
		template <typename SectionType>
		TSharedPtr<SectionType> GetSectionAs(const FRealtimeMeshSectionKey& SectionKey) const
		{
			return StaticCastSharedPtr<SectionType>(GetSection(SectionKey));
		}

		TFuture<ERealtimeMeshProxyUpdateStatus> InitializeLODs(const TFixedLODArray<FRealtimeMeshLODConfig>& InLODConfigs);
		void InitializeLODs(FRealtimeMeshProxyCommandBatch& Commands, const TFixedLODArray<FRealtimeMeshLODConfig>& InLODConfigs);
		TFuture<ERealtimeMeshProxyUpdateStatus> AddLOD(const FRealtimeMeshLODConfig& LODConfig, FRealtimeMeshLODKey* OutLODKey = nullptr);
		virtual void AddLOD(FRealtimeMeshProxyCommandBatch& Commands, const FRealtimeMeshLODConfig& LODConfig, FRealtimeMeshLODKey* OutLODKey = nullptr);
		TFuture<ERealtimeMeshProxyUpdateStatus> RemoveTrailingLOD(FRealtimeMeshLODKey* OutNewLastLODKey = nullptr);
		virtual void RemoveTrailingLOD(FRealtimeMeshProxyCommandBatch& Commands, FRealtimeMeshLODKey* OutNewLastLODKey = nullptr);

		TFuture<ERealtimeMeshProxyUpdateStatus> Reset(bool bRemoveRenderProxy = false);
		virtual void Reset(FRealtimeMeshProxyCommandBatch& Commands, bool bRemoveRenderProxy = false);

		virtual bool Serialize(FArchive& Ar);

		virtual void MarkRenderStateDirty(bool bShouldRecreateProxies)
		{
			SharedResources->BroadcastMeshRenderDataChanged();
			if (bShouldRecreateProxies)
			{
				SharedResources->BroadcastMeshRequestedProxyRecreate();
			}
		}

		bool HasRenderProxy() const;
		FRealtimeMeshProxyPtr GetRenderProxy(bool bCreateIfNotExists = false) const;

		virtual void InitializeProxy(FRealtimeMeshProxyCommandBatch& Commands) const;

	protected:
		virtual void ProcessEndOfFrameUpdates()
		{
		}

		/*TFuture<ERealtimeMeshProxyUpdateStatus> ApplyStateUpdate(FRealtimeMeshUpdateContext& Update);*/
		void HandleLODBoundsChanged(const FRealtimeMeshLODKey& LODKey);

		FRealtimeMeshProxyRef CreateRenderProxy(bool bForceRecreate = false) const;
		virtual FBoxSphereBounds3f CalculateBounds() const;

		TFuture<ERealtimeMeshCollisionUpdateResult> UpdateCollision(FRealtimeMeshCollisionData&& InCollisionData);

		friend class URealtimeMesh;
	};
}
