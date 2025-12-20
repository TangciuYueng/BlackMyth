// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/BMShopComponent.h"
#include "Core/BMDataSubsystem.h"
#include "Data/BMItemData.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UBMShopComponent::UBMShopComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 商店组件无需Tick，保持轻量
	PrimaryComponentTick.bCanEverTick = false;
}

void UBMShopComponent::BeginPlay()
{
	Super::BeginPlay();
	// 初始化运行时货架并同步一次UI
	RebuildRuntimeShelf();
	BroadcastShopChanged();
}

void UBMShopComponent::RebuildRuntimeShelf()
{
	// 将可编辑货架复制到集合中，方便快速校验售卖范围
	AvailableItems.Reset();
	for (const FBMShopSlot& Slot : DefaultShelf)
	{
		if (Slot.ItemID == NAME_None)
		{
			continue;
		}

		AvailableItems.Add(Slot.ItemID);
	}
}

bool UBMShopComponent::PurchaseItem(FName ItemID, int32 Count)
{
	// 先校验参数，并确保商店确实在售该物品
	if (ItemID == NAME_None || Count <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMShopComponent::PurchaseItem - invalid params"));
		return false;
	}

	if (!AvailableItems.Contains(ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMShopComponent::PurchaseItem - item %s not sold here"), *ItemID.ToString());
		return false;
	}

	const FBMItemData* ItemData = GetItemData(ItemID);
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMShopComponent::PurchaseItem - missing item data for %s"), *ItemID.ToString());
		return false;
	}

	const int32 CurrentCount = GetWarehouseCount(ItemID);
	if (ItemData->MaxStack > 0 && (CurrentCount + Count) > ItemData->MaxStack)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMShopComponent::PurchaseItem - %s reached MaxStack (%d), current %d, request %d"),
			*ItemID.ToString(), ItemData->MaxStack, CurrentCount, Count);
		return false;
	}

	const FBMShopSlot* SlotDefinition = FindSlotDefinition(ItemID);
	const float PricePerUnit = ResolvePrice(*ItemData, SlotDefinition);

	int32& StoredCount = WarehouseItems.FindOrAdd(ItemID);
	StoredCount += Count;

	UE_LOG(LogTemp, Log, TEXT("UBMShopComponent::PurchaseItem - bought %d of %s, unit price %.2f, total %.2f"),
		Count, *ItemID.ToString(), PricePerUnit, PricePerUnit * Count);

	BroadcastShopChanged();
	return true;
}

int32 UBMShopComponent::GetWarehouseCount(FName ItemID) const
{
	if (const int32* CountPtr = WarehouseItems.Find(ItemID))
	{
		return *CountPtr;
	}
	return 0;
}

TArray<FBMShopSlot> UBMShopComponent::GetShelfSnapshot() const
{
	return DefaultShelf;
}

const FBMItemData* UBMShopComponent::GetItemData(FName ItemID) const
{
	UBMDataSubsystem* DataSubsystem = GetDataSubsystem();
	return DataSubsystem ? DataSubsystem->GetItemData(ItemID) : nullptr;
}

const FBMShopSlot* UBMShopComponent::FindSlotDefinition(FName ItemID) const
{
	return DefaultShelf.FindByPredicate([ItemID](const FBMShopSlot& Slot)
	{
		return Slot.ItemID == ItemID;
	});
}

float UBMShopComponent::ResolvePrice(const FBMItemData& ItemData, const FBMShopSlot* SlotDefinition) const
{
	if (SlotDefinition && SlotDefinition->PriceOverride >= 0.f)
	{
		return SlotDefinition->PriceOverride;
	}
	return ItemData.Price;
}

void UBMShopComponent::BroadcastShopChanged()
{
	OnShopChanged.Broadcast();
}

UBMDataSubsystem* UBMShopComponent::GetDataSubsystem() const
{
	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UBMDataSubsystem>();
		}
	}
	return nullptr;
}
