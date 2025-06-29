// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrestre/Core/Entity/EntityItem.h"
#include "Terrestre/Core/Item/ItemBase.h"
// Sets default values
AEntityItem::AEntityItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static mesh"));
}

void AEntityItem::SetSpawnParams(int32 ItemCount, int32 ItemID)
{
	if (ItemID > 0 && ItemCount > 0)
	{
		const auto _BoundItem = NewObject<UItemBase>(UItemBase::StaticClass());
		_BoundItem->SetItemCount(ItemCount);
		_BoundItem->SetItemID(ItemID);
		UpdateBoundItem(_BoundItem);
	}
}

AEntityItem::~AEntityItem()
{
	BoundItem = nullptr;
}

void AEntityItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AEntityItem::UpdateBoundItem(UItemBase* _boundItem)
{
	BoundItem = _boundItem;
	if (BoundItem)
	{
		const FItemData* itemData = GEngine->GetEngineSubsystem<UItemDataSubsystem>()->GetItemData(_boundItem->GetItemID());
		ItemMesh->SetStaticMesh(itemData->StaticMesh.LoadSynchronous());
	}
	ItemCountInThisStack = _boundItem->GetItemCount();
}

// Called when the game starts or when spawned
void AEntityItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEntityItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

