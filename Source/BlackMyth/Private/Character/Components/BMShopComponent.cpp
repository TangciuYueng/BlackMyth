// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/BMShopComponent.h"
#include "Core/BMDataSubsystem.h"
#include "Data/BMItemData.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

/*
 * @brief Constructor of the UBMShopComponent class
 * @param ObjectInitializer The object initializer
 */
UBMShopComponent::UBMShopComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// �̵��������Tick����������
	PrimaryComponentTick.bCanEverTick = false;
}

/*
 * @brief Begin play, it rebuilds the runtime shelf and broadcasts the shop changed
 */
void UBMShopComponent::BeginPlay()
{
	Super::BeginPlay();
	// ��ʼ������ʱ���ܲ�ͬ��һ��UI
	RebuildRuntimeShelf();
	BroadcastShopChanged();
}

/*
 * @brief Rebuild runtime shelf, it rebuilds the runtime shelf
 */
void UBMShopComponent::RebuildRuntimeShelf()
{
	// ���ɱ༭���ܸ��Ƶ������У��������У��������Χ
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

/*
 * @brief Purchase item, it purchases the item
 * @param ItemID The item ID
 * @param Count The count
 * @return True if the item is purchased, false otherwise
 */
bool UBMShopComponent::PurchaseItem(FName ItemID, int32 Count)
{
	// ��У���������ȷ���̵�ȷʵ���۸���Ʒ
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

/*
 * @brief Get warehouse count, it gets the warehouse count
 * @param ItemID The item ID
 * @return The warehouse count
 */
int32 UBMShopComponent::GetWarehouseCount(FName ItemID) const
{
	if (const int32* CountPtr = WarehouseItems.Find(ItemID))
	{
		return *CountPtr;
	}
	return 0;
}

/*
 * @brief Get shelf snapshot, it gets the shelf snapshot
 * @return The shelf snapshot
 */
TArray<FBMShopSlot> UBMShopComponent::GetShelfSnapshot() const
{
	return DefaultShelf;
}

/*
 * @brief Get item data, it gets the item data
 * @param ItemID The item ID
 * @return The item data
 */
const FBMItemData* UBMShopComponent::GetItemData(FName ItemID) const
{
	UBMDataSubsystem* DataSubsystem = GetDataSubsystem();
	return DataSubsystem ? DataSubsystem->GetItemData(ItemID) : nullptr;
}

/*
 * @brief Find slot definition, it finds the slot definition
 * @param ItemID The item ID
 * @return The slot definition
 */
const FBMShopSlot* UBMShopComponent::FindSlotDefinition(FName ItemID) const
{
	return DefaultShelf.FindByPredicate([ItemID](const FBMShopSlot& Slot)
	{
		return Slot.ItemID == ItemID;
	});
}

/*
 * @brief Resolve price, it resolves the price
 * @param ItemData The item data
 * @param SlotDefinition The slot definition
 * @return The price
 */
float UBMShopComponent::ResolvePrice(const FBMItemData& ItemData, const FBMShopSlot* SlotDefinition) const
{
	if (SlotDefinition && SlotDefinition->PriceOverride >= 0.f)
	{
		return SlotDefinition->PriceOverride;
	}
	return ItemData.Price;
}

/*
 * @brief Broadcast shop changed, it broadcasts the shop changed
 */
void UBMShopComponent::BroadcastShopChanged()
{
	OnShopChanged.Broadcast();
}

/*
 * @brief Get data subsystem, it gets the data subsystem
 * @return The data subsystem
 */
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
