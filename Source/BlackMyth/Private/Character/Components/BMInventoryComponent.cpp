// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Components/BMInventoryComponent.h"
#include "Core/BMDataSubsystem.h"
#include "Data/BMItemData.h"
#include "Engine/Engine.h"

UBMInventoryComponent::UBMInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 背包组件不需要每帧更新，禁用Tick以提高性能
	PrimaryComponentTick.bCanEverTick = false;

	// 初始化默认值
	Capacity = 20;
	Currency = 0;
	Items.Empty();
}

void UBMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UBMInventoryComponent::AddItem(FName ItemID, int32 Count)
{
	// 检查ItemID是否有效
	if (ItemID == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::AddItem - 无效的ItemID"));
		return false;
	}

	// 检查Count是否为正数
	if (Count <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::AddItem - Count必须大于0，当前值: %d"), Count);
		return false;
	}

	// 验证ItemID是否在数据表中存在
	const FBMItemData* ItemData = GetItemData(ItemID);
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::AddItem - 物品 %s 在数据表中不存在"), *ItemID.ToString());
		return false;
	}

	// 检查物品是否已存在
	if (Items.Contains(ItemID))
	{
		// 物品已存在，检查堆叠限制
		int32 CurrentCount = Items[ItemID];
		int32 MaxStack = ItemData->MaxStack;
		
		// 检查是否会超过最大堆叠数
		if (MaxStack > 0 && (CurrentCount + Count) > MaxStack)
		{
			// 计算可以添加的数量
			int32 CanAdd = MaxStack - CurrentCount;
			if (CanAdd <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::AddItem - 物品 %s 已达到最大堆叠数 %d，无法添加更多"), 
					*ItemID.ToString(), MaxStack);
				return false;
			}
			
			// 只添加可以堆叠的数量
			Items[ItemID] = MaxStack;
			UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::AddItem - 物品 %s 已达到最大堆叠数，只添加了 %d 个（请求 %d 个）"), 
				*ItemID.ToString(), CanAdd, Count);
			return true;
		}
		
		// 可以正常堆叠，直接增加数量
		Items[ItemID] += Count;
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::AddItem - 物品 %s 数量增加 %d，当前数量: %d"), 
			*ItemID.ToString(), Count, Items[ItemID]);
		return true;
	}
	else
	{
		// 物品不存在，需要检查容量
		if (IsFull())
		{
			UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::AddItem - 背包已满，无法添加新物品 %s"), *ItemID.ToString());
			return false;
		}

		// 检查初始数量是否超过最大堆叠数
		int32 MaxStack = ItemData->MaxStack;
		int32 ActualCount = Count;
		if (MaxStack > 0 && Count > MaxStack)
		{
			ActualCount = MaxStack;
			UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::AddItem - 物品 %s 初始数量 %d 超过最大堆叠数 %d，设置为 %d"), 
				*ItemID.ToString(), Count, MaxStack, ActualCount);
		}

		// 容量未满，创建新条目
		Items.Add(ItemID, ActualCount);
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::AddItem - 添加新物品 %s，数量: %d"), 
			*ItemID.ToString(), ActualCount);
		return true;
	}
}

bool UBMInventoryComponent::RemoveItem(FName ItemID, int32 Count)
{
	// 参数验证：检查ItemID是否有效
	if (ItemID == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::RemoveItem - 无效的ItemID"));
		return false;
	}

	// 参数验证：检查Count是否为正数
	if (Count <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::RemoveItem - Count必须大于0，当前值: %d"), Count);
		return false;
	}

	// 检查物品是否存在
	if (!Items.Contains(ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::RemoveItem - 物品 %s 不存在"), *ItemID.ToString());
		return false;
	}

	// 检查数量是否足够
	int32 CurrentCount = Items[ItemID];
	if (CurrentCount < Count)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::RemoveItem - 物品 %s 数量不足，当前: %d，需要: %d"), 
			*ItemID.ToString(), CurrentCount, Count);
		return false;
	}

	// 减少数量
	CurrentCount -= Count;
	
	// 如果数量归零，从Map中移除该Key
	if (CurrentCount <= 0)
	{
		Items.Remove(ItemID);
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::RemoveItem - 物品 %s 已完全移除"), *ItemID.ToString());
	}
	else
	{
		Items[ItemID] = CurrentCount;
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::RemoveItem - 物品 %s 数量减少 %d，剩余数量: %d"), 
			*ItemID.ToString(), Count, CurrentCount);
	}

	return true;
}

bool UBMInventoryComponent::HasItem(FName ItemID) const
{
	// 检查ItemID是否有效
	if (ItemID == NAME_None)
	{
		return false;
	}

	// 查询Items Map中是否存在该Key
	return Items.Contains(ItemID);
}

int32 UBMInventoryComponent::GetItemCount(FName ItemID) const
{
	// 检查ItemID是否有效
	if (ItemID == NAME_None)
	{
		return 0;
	}

	// 如果物品存在，返回对应的数量；否则返回0
	const int32* CountPtr = Items.Find(ItemID);
	return CountPtr ? *CountPtr : 0;
}

bool UBMInventoryComponent::CanAfford(int32 Cost) const
{
	// 检查Cost是否有效
	if (Cost < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::CanAfford - Cost不能为负数，当前值: %d"), Cost);
		return false;
	}

	// 判断货币是否大于或等于Cost
	return Currency >= Cost;
}

bool UBMInventoryComponent::AddCurrency(int32 Amount)
{
	// 参数验证：检查Amount是否为正数
	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::AddCurrency - Amount必须大于0，当前值: %d"), Amount);
		return false;
	}

	// 添加货币（注意：这里不检查溢出，因为int32最大值很大）
	Currency += Amount;
	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::AddCurrency - 添加货币 %d，当前货币: %d"), Amount, Currency);
	return true;
}

bool UBMInventoryComponent::SpendCurrency(int32 Amount)
{
	// 参数验证：检查Amount是否为正数
	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::SpendCurrency - Amount必须大于0，当前值: %d"), Amount);
		return false;
	}

	// 检查货币是否足够
	if (!CanAfford(Amount))
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::SpendCurrency - 货币不足，当前: %d，需要: %d"), Currency, Amount);
		return false;
	}

	// 消费货币
	Currency -= Amount;
	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::SpendCurrency - 消费货币 %d，剩余货币: %d"), Amount, Currency);
	return true;
}

int32 UBMInventoryComponent::GetItemTypeCount() const
{
	// 返回当前背包中不同物品的种类数量
	return Items.Num();
}

int32 UBMInventoryComponent::GetRemainingCapacity() const
{
	// 计算剩余容量 = 总容量 - 当前物品种类数量
	int32 Remaining = Capacity - GetItemTypeCount();
	return FMath::Max(0, Remaining); // 确保不为负数
}

bool UBMInventoryComponent::IsFull() const
{
	// 检查当前物品种类数量是否达到容量上限
	return GetItemTypeCount() >= Capacity;
}

void UBMInventoryComponent::SetCapacity(int32 NewCapacity)
{
	// 参数验证：检查NewCapacity是否为正数
	if (NewCapacity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::SetCapacity - NewCapacity必须大于0，当前值: %d"), NewCapacity);
		return;
	}

	// 如果新容量小于当前物品种类数量，需要移除多余的物品
	if (NewCapacity < GetItemTypeCount())
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::SetCapacity - 新容量 %d 小于当前物品种类数量 %d，将清空超出容量的物品"), 
			NewCapacity, GetItemTypeCount());
		
		// 这里可以选择保留前N个物品，或者清空所有物品
		// 为了简单起见，这里只记录警告，不自动移除物品
		// 如果需要，可以在这里实现更复杂的逻辑
	}

	Capacity = NewCapacity;
	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::SetCapacity - 设置新容量: %d"), Capacity);
}

void UBMInventoryComponent::ClearInventory()
{
	// 清空所有物品
	int32 ItemCount = Items.Num();
	Items.Empty();
	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::ClearInventory - 已清空背包，移除了 %d 种物品"), ItemCount);
}

// ==================== 物品信息查询方法实现 ====================

FText UBMInventoryComponent::GetItemName(FName ItemID) const
{
	const FBMItemData* ItemData = GetItemData(ItemID);
	if (ItemData)
	{
		return ItemData->Name;
	}
	return FText::GetEmpty();
}

FSoftObjectPath UBMInventoryComponent::GetItemIcon(FName ItemID) const
{
	const FBMItemData* ItemData = GetItemData(ItemID);
	if (ItemData)
	{
		return ItemData->IconPath;
	}
	return FSoftObjectPath();
}

float UBMInventoryComponent::GetItemPrice(FName ItemID) const
{
	const FBMItemData* ItemData = GetItemData(ItemID);
	if (ItemData)
	{
		return ItemData->Price;
	}
	return 0.0f;
}

int32 UBMInventoryComponent::GetItemMaxStack(FName ItemID) const
{
	const FBMItemData* ItemData = GetItemData(ItemID);
	if (ItemData)
	{
		return ItemData->MaxStack;
	}
	return 0;
}

bool UBMInventoryComponent::CanStackMore(FName ItemID, int32 AdditionalCount) const
{
	// 检查ItemID是否有效
	if (ItemID == NAME_None)
	{
		return false;
	}

	// 获取物品数据
	const FBMItemData* ItemData = GetItemData(ItemID);
	if (!ItemData)
	{
		return false;
	}

	// 如果物品不在背包中，可以添加（只要不超过最大堆叠数）
	if (!Items.Contains(ItemID))
	{
		int32 MaxStack = ItemData->MaxStack;
		// MaxStack <= 0 表示无限制
		return MaxStack <= 0 || AdditionalCount <= MaxStack;
	}

	// 物品已存在，检查是否可以继续堆叠
	int32 CurrentCount = Items[ItemID];
	int32 MaxStack = ItemData->MaxStack;
	
	// MaxStack <= 0 表示无限制
	if (MaxStack <= 0)
	{
		return true;
	}

	// 检查添加后是否超过最大堆叠数
	return (CurrentCount + AdditionalCount) <= MaxStack;
}

const FBMItemData* UBMInventoryComponent::GetItemData(FName ItemID) const
{
	// 检查ItemID是否有效
	if (ItemID == NAME_None)
	{
		return nullptr;
	}

	// 获取数据子系统
	UBMDataSubsystem* DataSubsystem = GetDataSubsystem();
	if (!DataSubsystem)
	{
		return nullptr;
	}

	// 从数据子系统获取物品数据
	return DataSubsystem->GetItemData(ItemID);
}

UBMDataSubsystem* UBMInventoryComponent::GetDataSubsystem() const
{
	// 从World获取GameInstance，再获取DataSubsystem
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UBMDataSubsystem>();
		}
	}
	return nullptr;
}

