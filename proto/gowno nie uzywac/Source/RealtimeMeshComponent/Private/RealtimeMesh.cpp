// Copyright TriAxis Games, L.L.C. All Rights Reserved.

#include "RealtimeMesh.h"
#include "RealtimeMeshComponent.h"
#include "Data/RealtimeMeshData.h"
#include "Data/RealtimeMeshLOD.h"
#include "Interface_CollisionDataProviderCore.h"
#include "Misc/LazySingleton.h"
#include "PhysicsEngine/BodySetup.h"
#include "Logging/MessageLog.h"
#if RMC_ENGINE_ABOVE_5_2
#include "Engine/World.h"
#endif

#define LOCTEXT_NAMESPACE "RealtimeMesh"

DECLARE_DWORD_COUNTER_STAT(TEXT("RealtimeMeshDelayedActions - Updated Actors"), STAT_RealtimeMeshDelayedActions_UpdatedActors, STATGROUP_RealtimeMesh);
DECLARE_CYCLE_STAT(TEXT("RealtimeMeshDelayedActions - Tick"), STAT_RealtimeMeshDelayedActions_Tick, STATGROUP_RealtimeMesh);
DECLARE_CYCLE_STAT(TEXT("RealtimeMeshDelayedActions - Initialize"), STAT_RealtimeMesh_Initialize, STATGROUP_RealtimeMesh);
DECLARE_CYCLE_STAT(TEXT("RealtimeMeshDelayedActions - Get Physics TriMesh"), STAT_RealtimeMesh_GetPhysicsTriMesh, STATGROUP_RealtimeMesh);
DECLARE_CYCLE_STAT(TEXT("RealtimeMeshDelayedActions - Has Physics TriMesh"), STAT_RealtimeMesh_HasPhysicsTriMesh, STATGROUP_RealtimeMesh);
DECLARE_CYCLE_STAT(TEXT("RealtimeMeshDelayedActions - Update Collision"), STAT_RealtimeMesh_UpdateCollision, STATGROUP_RealtimeMesh);
DECLARE_CYCLE_STAT(TEXT("RealtimeMeshDelayedActions - Finish Collision Async Cook"), STAT_RealtimeMesh_FinishCollisionAsyncCook, STATGROUP_RealtimeMesh);
DECLARE_CYCLE_STAT(TEXT("RealtimeMeshDelayedActions - Finalize Collision Cooked Data"), STAT_RealtimeMesh_FinalizeCollisionCookedData, STATGROUP_RealtimeMesh);


//////////////////////////////////////////////////////////////////////////
//	URealtimeMesh

URealtimeMesh::URealtimeMesh(const FObjectInitializer& ObjectInitializer)
	: UObject(ObjectInitializer)
	, BodySetup(nullptr)
	, PendingBodySetup(nullptr)
	, CollisionUpdateVersionCounter(0)
	, CurrentCollisionVersion(INDEX_NONE)
{
}

void URealtimeMesh::BroadcastCollisionBodyUpdatedEvent(UBodySetup* NewBodySetup)
{
	CollisionBodyUpdatedEvent.Broadcast(this, NewBodySetup);
}

void URealtimeMesh::Initialize(const TSharedRef<RealtimeMesh::FRealtimeMeshSharedResources>& InSharedResources)
{
	if (SharedResources)
	{
		SharedResources->OnMeshBoundsChanged().RemoveAll(this);
		SharedResources->OnMeshRenderDataChanged().RemoveAll(this);
	}

	SharedResources = InSharedResources;

	SharedResources->OnMeshBoundsChanged().AddUObject(this, &URealtimeMesh::HandleBoundsUpdated);
	SharedResources->OnMeshRenderDataChanged().AddUObject(this, &URealtimeMesh::HandleMeshRenderingDataChanged);

	SharedResources->GetEndOfFrameRequestHandler() = RealtimeMesh::FRealtimeMeshRequestEndOfFrameUpdateDelegate::CreateUObject(this, &URealtimeMesh::MarkForEndOfFrameUpdate);
	SharedResources->GetCollisionUpdateHandler() = RealtimeMesh::FRealtimeMeshCollisionUpdateDelegate::CreateUObject(this, &URealtimeMesh::InitiateCollisionUpdate);

	MeshRef = SharedResources->CreateRealtimeMesh();
	SharedResources->SetOwnerMesh(MeshRef.ToSharedRef());
}


void URealtimeMesh::Reset(bool bCreateNewMeshData)
{
	UE_LOG(LogTemp, Warning, TEXT("RM Resetting... %s"), *GetName());
	if (!bCreateNewMeshData)
	{
		GetMesh()->Reset();
	}
	else
	{
		Initialize(SharedResources->CreateSharedResources());
	}

	BodySetup = nullptr;

	BroadcastBoundsChangedEvent();
	BroadcastRenderDataChangedEvent(true);
	BroadcastCollisionBodyUpdatedEvent(nullptr);
}

FBoxSphereBounds URealtimeMesh::GetLocalBounds() const
{
	return FBoxSphereBounds(GetMesh()->GetLocalBounds());
}


FRealtimeMeshLODKey URealtimeMesh::AddLOD(const FRealtimeMeshLODConfig& Config)
{
	FRealtimeMeshLODKey LODKey;
	GetMesh()->AddLOD(Config, &LODKey);
	return LODKey;
}

void URealtimeMesh::UpdateLODConfig(FRealtimeMeshLODKey LODKey, const FRealtimeMeshLODConfig& Config)
{
	if (const auto LOD = GetMesh()->GetLOD(LODKey))
	{
		LOD->UpdateConfig(Config);
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("UpdateLODConfig_InvalidLODKey", "UpdateLODConfig: Invalid LOD key {0}"), FText::FromString(LODKey.ToString())));
	}
}

void URealtimeMesh::RemoveTrailingLOD()
{
	GetMesh()->RemoveTrailingLOD();
}


void URealtimeMesh::SetupMaterialSlot(int32 MaterialSlot, FName SlotName, UMaterialInterface* InMaterial)
{
	// Does this slot already exist?
	if (SlotNameLookup.Contains(SlotName))
	{
		// If the indices match then just go with it
		if (SlotNameLookup[SlotName] == MaterialSlot)
		{
			MaterialSlots[SlotNameLookup[SlotName]].Material = InMaterial;
		}
		else
		{
			MaterialSlots[SlotNameLookup[SlotName]].SlotName = NAME_None;
		}
	}

	if (!MaterialSlots.IsValidIndex(MaterialSlot))
	{
		MaterialSlots.SetNum(MaterialSlot + 1);
	}
	MaterialSlots[MaterialSlot] = FRealtimeMeshMaterialSlot(SlotName, InMaterial);
	SlotNameLookup.Add(SlotName, MaterialSlots.Num() - 1);

	BroadcastRenderDataChangedEvent(true);
}

int32 URealtimeMesh::GetMaterialIndex(FName MaterialSlotName) const
{
	const int32* SlotIndex = SlotNameLookup.Find(MaterialSlotName);
	return SlotIndex ? *SlotIndex : INDEX_NONE;
}

bool URealtimeMesh::IsMaterialSlotNameValid(FName MaterialSlotName) const
{
	return SlotNameLookup.Contains(MaterialSlotName);
}

FRealtimeMeshMaterialSlot URealtimeMesh::GetMaterialSlot(int32 SlotIndex) const
{
	return MaterialSlots[SlotIndex];
}

int32 URealtimeMesh::GetNumMaterials() const
{
	return MaterialSlots.Num();
}

TArray<FName> URealtimeMesh::GetMaterialSlotNames() const
{
	TArray<FName> OutNames;
	SlotNameLookup.GetKeys(OutNames);
	return OutNames;
}

TArray<FRealtimeMeshMaterialSlot> URealtimeMesh::GetMaterialSlots() const
{
	return MaterialSlots;
}

UMaterialInterface* URealtimeMesh::GetMaterial(int32 SlotIndex) const
{
	if (MaterialSlots.IsValidIndex(SlotIndex))
	{
		return MaterialSlots[SlotIndex].Material;
	}
	return nullptr;
}

void URealtimeMesh::PostInitProperties()
{
	UObject::PostInitProperties();

	if (!IsTemplate() && SharedResources)
	{
		SharedResources->SetMeshName(this->GetFName());
	}
}

void URealtimeMesh::BeginDestroy()
{
	Super::BeginDestroy();
}

void URealtimeMesh::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (!IsTemplate())
	{
		Ar.UsingCustomVersion(RealtimeMesh::FRealtimeMeshVersion::GUID);

		// Serialize the mesh data
		GetMesh()->Serialize(Ar);
	}
}

void URealtimeMesh::PostDuplicate(bool bDuplicateForPIE)
{
	UObject::PostDuplicate(bDuplicateForPIE);
}

bool URealtimeMesh::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	SCOPE_CYCLE_COUNTER(STAT_RealtimeMesh_GetPhysicsTriMesh);

	RealtimeMesh::FRealtimeMeshScopeGuardRead ScopeGuard(SharedResources->GetGuard());
	if (PendingCollisionUpdate.IsSet())
	{
		const auto& MeshData = PendingCollisionUpdate.GetValue().TriMeshData;

		CollisionData->Vertices = MeshData.GetVertices();
		CollisionData->Indices = MeshData.GetTriangles();
		CollisionData->UVs = MeshData.GetUVs();
		CollisionData->MaterialIndices = MeshData.GetMaterials();


		CollisionData->bFlipNormals = true;
		CollisionData->bDeformableMesh = false;
		CollisionData->bFastCook = PendingCollisionUpdate.GetValue().bFastCook;
		CollisionData->bDisableActiveEdgePrecompute = false;
		return true;
	}

	return false;
}

bool URealtimeMesh::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	SCOPE_CYCLE_COUNTER(STAT_RealtimeMesh_HasPhysicsTriMesh);

	return PendingCollisionUpdate.IsSet() && PendingCollisionUpdate.GetValue().TriMeshData.GetTriangles().Num() > 0;
}

void URealtimeMesh::InitiateCollisionUpdate(const TSharedRef<TPromise<ERealtimeMeshCollisionUpdateResult>>& Promise, const TSharedRef<FRealtimeMeshCollisionData>& CollisionUpdate,
                                            bool bForceSyncUpdate)
{
	check(IsInGameThread());
	check(SharedResources && MeshRef);

	RealtimeMesh::FRealtimeMeshScopeGuardWrite Guard(SharedResources->GetGuard());

	const int32 UpdateKey = CollisionUpdateVersionCounter++;
	PendingCollisionUpdate = {MoveTemp(CollisionUpdate->ComplexGeometry), UpdateKey};

	UBodySetup* NewBodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public : RF_NoFlags));
	NewBodySetup->BodySetupGuid = FGuid::NewGuid();
	CollisionUpdate->SimpleGeometry.CopyToBodySetup(BodySetup);

	NewBodySetup->bGenerateMirroredCollision = false;
	NewBodySetup->bDoubleSidedGeometry = true;
	NewBodySetup->CollisionTraceFlag = CollisionUpdate->Config.bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

	// Abort any pending update.
	if (PendingBodySetup)
	{
		PendingBodySetup->AbortPhysicsMeshAsyncCreation();
		PendingBodySetup = nullptr;
	}

	if (!bForceSyncUpdate && (!GetWorld() || GetWorld()->IsGameWorld()) && CollisionUpdate->Config.bUseAsyncCook)
	{
		// Copy source info and reset pending
		PendingBodySetup = NewBodySetup;

		// Kick the cook off asynchronously
		NewBodySetup->CreatePhysicsMeshesAsync(
			FOnAsyncPhysicsCookFinished::CreateUObject(this, &URealtimeMesh::FinishPhysicsAsyncCook, Promise, NewBodySetup, UpdateKey));
	}
	else
	{
		// Update meshes
		NewBodySetup->bHasCookedCollisionData = true;
		NewBodySetup->InvalidatePhysicsData();
		NewBodySetup->CreatePhysicsMeshes();

		BodySetup = NewBodySetup;

		Promise->SetValue(ERealtimeMeshCollisionUpdateResult::Updated);
	}
}

// ReSharper disable once CppPassValueParameterByConstReference
void URealtimeMesh::FinishPhysicsAsyncCook(bool bSuccess, TSharedRef<TPromise<ERealtimeMeshCollisionUpdateResult>> Promise, UBodySetup* FinishedBodySetup, int32 UpdateKey)
{
	check(IsInGameThread());
	check(SharedResources && MeshRef);

	RealtimeMesh::FRealtimeMeshScopeGuardWrite Guard(SharedResources->GetGuard());

	bool bSendEvent = false;

	// Apply body setup if newer and succeeded build
	if (bSuccess)
	{
		if (UpdateKey > CurrentCollisionVersion)
		{
			BodySetup = FinishedBodySetup;
			CurrentCollisionVersion = UpdateKey;
			Promise->SetValue(ERealtimeMeshCollisionUpdateResult::Updated);
			bSendEvent = true;
		}
		else
		{
			Promise->SetValue(ERealtimeMeshCollisionUpdateResult::Ignored);
		}
	}
	else
	{
		CurrentCollisionVersion = UpdateKey;
		Promise->SetValue(ERealtimeMeshCollisionUpdateResult::Error);
	}

	if (PendingBodySetup == FinishedBodySetup)
	{
		PendingBodySetup = nullptr;
	}

	Guard.Unlock();

	if (bSendEvent)
	{
		BroadcastCollisionBodyUpdatedEvent(BodySetup);
	}
}


void URealtimeMesh::HandleBoundsUpdated()
{
	BroadcastBoundsChangedEvent();
}

void URealtimeMesh::HandleMeshRenderingDataChanged()
{
	Modify(true);
	BroadcastRenderDataChangedEvent(true);
}


struct FRealtimeMeshEndOfFrameUpdateManager
{
private:
	FCriticalSection SyncRoot;
	TSet<TWeakObjectPtr<URealtimeMesh>> MeshesToUpdate;
	FDelegateHandle EndOfFrameUpdateHandle;

	void OnPreSendAllEndOfFrameUpdates(UWorld* World)
	{
		SyncRoot.Lock();
		auto MeshesCopy = MoveTemp(MeshesToUpdate);
		SyncRoot.Unlock();

		for (const auto& Mesh : MeshesCopy)
		{
			if (Mesh.IsValid())
			{
				Mesh->ProcessEndOfFrameUpdates();
			}
		}
	}

public:
	~FRealtimeMeshEndOfFrameUpdateManager()
	{
		if (EndOfFrameUpdateHandle.IsValid())
		{
			FWorldDelegates::OnWorldPreSendAllEndOfFrameUpdates.Remove(EndOfFrameUpdateHandle);
			EndOfFrameUpdateHandle.Reset();
		}
	}

	void MarkComponentForUpdate(URealtimeMesh* RealtimeMesh)
	{
		FScopeLock Lock(&SyncRoot);
		if (!EndOfFrameUpdateHandle.IsValid())
		{
			EndOfFrameUpdateHandle = FWorldDelegates::OnWorldPreSendAllEndOfFrameUpdates.AddRaw(this, &FRealtimeMeshEndOfFrameUpdateManager::OnPreSendAllEndOfFrameUpdates);
		}
		MeshesToUpdate.Add(RealtimeMesh);
	}

	void ClearComponentForUpdate(URealtimeMesh* RealtimeMesh)
	{
		FScopeLock Lock(&SyncRoot);
		MeshesToUpdate.Remove(RealtimeMesh);
	}

	static FRealtimeMeshEndOfFrameUpdateManager& Get()
	{
		return TLazySingleton<FRealtimeMeshEndOfFrameUpdateManager>::Get();
	}
};


void URealtimeMesh::ProcessEndOfFrameUpdates()
{
	if (MeshRef)
	{
		MeshRef->ProcessEndOfFrameUpdates();
	}
}

void URealtimeMesh::MarkForEndOfFrameUpdate()
{
	FRealtimeMeshEndOfFrameUpdateManager::Get().MarkComponentForUpdate(this);
}

#undef LOCTEXT_NAMESPACE
