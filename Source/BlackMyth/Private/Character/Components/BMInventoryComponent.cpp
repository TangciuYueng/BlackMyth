// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Components/BMInventoryComponent.h"
#include "Character/Components/BMStatsComponent.h"
#include "Core/BMDataSubsystem.h"
#include "Data/BMItemData.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "Engine/Engine.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UI/UBMInventoryWidget.h"

/*
 * @brief Constructor of the UBMInventoryComponent class
 * @param ObjectInitializer The object initializer
 */
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

/*
 * @brief Begin play, it starts the test auto add currency
 */
void UBMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	StartTestAutoAddCurrency();
}

/*
 * @brief Start test auto add currency, it starts the test auto add currency
 */
void UBMInventoryComponent::StartTestAutoAddCurrency()
{
	if (!bTestAutoAddCurrency)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	/*
	 * @brief Set timer for test auto add currency, it sets the timer for test auto add currency
	 * @param Interval The interval
	 */
	const float Interval = FMath::Max(0.01f, TestAutoAddCurrencyIntervalSeconds);
	World->GetTimerManager().SetTimer(
		TestAutoAddCurrencyTimer,
		this,
		&UBMInventoryComponent::HandleTestAutoAddCurrencyTick,
		Interval,
		true);

	HandleTestAutoAddCurrencyTick();
}

/*
 * @brief Handle test auto add currency tick, it handles the test auto add currency tick
 */
void UBMInventoryComponent::HandleTestAutoAddCurrencyTick()
{
	if (!bTestAutoAddCurrency)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TestAutoAddCurrencyTimer);
		}
		return;
	}

	const int32 Amount = FMath::Max(1, TestAutoAddCurrencyAmount);
	AddCurrency(Amount);
}

/*
 * @brief Add item, it adds the item to the inventory
 * @param ItemID The item ID
 * @param Count The count
 * @return True if the item is added, false otherwise
 */
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
		UBMDataSubsystem* DataSubsystem = GetDataSubsystem();
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::AddItem - 物品 %s 在数据表中不存在 (ItemTable=%s)"),
			*ItemID.ToString(),
			DataSubsystem ? *DataSubsystem->GetItemTablePathDebug() : TEXT("null"));
		return false;
	}

	bool bChanged = false;

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
		}
		else
		{
			// 可以正常堆叠，直接增加数量
			Items[ItemID] += Count;
			const int32 NewCount = Items[ItemID];
			UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::AddItem - 物品 %s 数量增加 %d，当前数量: %d"), 
				*ItemID.ToString(), Count, Items[ItemID]);
		}
		bChanged = true;
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
		bChanged = true;
	}

	if (bChanged)
	{
		// 玩家背包发生变化后即刻通知UI层刷新
		OnInventoryChanged.Broadcast();
	}
	return bChanged;
}

/*
 * @brief Remove item, it removes the item from the inventory
 * @param ItemID The item ID
 * @param Count The count
 * @return True if the item is removed, false otherwise
 */
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

	// 物品数量变化后广播，促使UI减少显示数量
	OnInventoryChanged.Broadcast();
	return true;
}

/*
 * @brief Use item, it uses the item
 * @param ItemID The item ID
 * @param Count The count
 * @return True if the item is used, false otherwise
 */
bool UBMInventoryComponent::UseItem(FName ItemID, int32 Count)
{
	if (Count <= 0)
	{
		return false;
	}

	if (GetItemCount(ItemID) < Count)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::UseItem - 物品 %s 数量不足"), *ItemID.ToString());
		
		// 构建通知消息
		FString ItemName = ItemID.ToString();
		ItemName.RemoveFromStart(TEXT("Item_"));
		FString NotificationMessage = FString::Printf(TEXT("Insufficient: %s (Need: %d, Have: %d)"), 
			*ItemName, Count, GetItemCount(ItemID));
		
		// 通过事件总线发送通知
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (UBMEventBusSubsystem* EventBus = GI->GetSubsystem<UBMEventBusSubsystem>())
				{
					EventBus->EmitNotify(FText::FromString(NotificationMessage));
				}
			}
		}
		
		return false;
	}

	const FBMItemData* ItemData = GetItemData(ItemID);
	if (!ItemData)
	{
		return false;
	}

	// 获取角色的Stats组件
	UBMStatsComponent* StatsComp = nullptr;
	if (AActor* Owner = GetOwner())
	{
		StatsComp = Owner->FindComponentByClass<UBMStatsComponent>();
	}

	if (!StatsComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::UseItem - 无法获取StatsComponent"));
		return false;
	}

	// 应用道具效果并记录效果描述
	TArray<FString> Effects;
	bool bEffectApplied = false;

	// 1. 最大生命值临时提升（MaxHp字段：15秒内临时提升到该值）
	if (ItemData->MaxHp > 0.f)
	{
		// 计算当前HP百分比
		const float CurrentMaxHp = StatsComp->GetStatBlock().MaxHP;
		const float CurrentHp = StatsComp->GetStatBlock().HP;
		const float HpPercentage = (CurrentMaxHp > 0.f) ? (CurrentHp / CurrentMaxHp) : 1.0f;

		// 添加临时最大生命值加成（15秒）
		constexpr float MaxHpBoostDuration = 15.f;
		const float NewMaxHp = ItemData->MaxHp;
		
		// Value 存储的是提升后的最大生命值
		StatsComp->AddBuff(EBMBuffType::MaxHpBoost, NewMaxHp, MaxHpBoostDuration);
		
		// 立即调整当前HP以保持百分比不变
		FBMStatBlock& Stats = StatsComp->GetStatBlockMutable();
		Stats.HP = NewMaxHp * HpPercentage;
		
		Effects.Add(FString::Printf(TEXT("Max HP: %.0f (15s)"), NewMaxHp));
		
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::UseItem - %s 临时提升最大生命值 %.1f -> %.1f (持续 %.1fs)，当前HP调整为 %.1f (%.1f%%)"), 
			*ItemID.ToString(), CurrentMaxHp, NewMaxHp, MaxHpBoostDuration, Stats.HP, HpPercentage * 100.f);
		
		bEffectApplied = true;
	}

	// 2. 立即回复HP（Hp字段：一次回复的血量百分比）
	if (ItemData->Hp > 0.f)
	{
		StatsComp->HealByPercent(ItemData->Hp);
		Effects.Add(FString::Printf(TEXT("+%.0f%% HP"), ItemData->Hp));
		
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::UseItem - %s 恢复 %.1f%% HP"), 
			*ItemID.ToString(), ItemData->Hp);
		bEffectApplied = true;
	}

	// 3. 攻击力加成（AttackPower字段：持续15秒的百分比攻击力加成）
	if (ItemData->AttackPower > 0.f)
	{
		// 攻击力加成持续15秒
		constexpr float AttackBoostDuration = 15.f;
		StatsComp->AddBuff(EBMBuffType::AttackBoost, ItemData->AttackPower, AttackBoostDuration);
		Effects.Add(FString::Printf(TEXT("+%.0f%% Attack (15s)"), ItemData->AttackPower));
		
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::UseItem - %s 添加 %.1f%% 攻击力加成，持续 %.1fs"), 
			*ItemID.ToString(), ItemData->AttackPower, AttackBoostDuration);
		bEffectApplied = true;
	}

	// 4. 处理额外功能（AdditionalFunction字段）
	switch (ItemData->AdditionalFunction)
	{
		case EBMItemFunction::Stamina:
		{
			// 加速回复耐力：2倍恢复速度，持续15秒
			constexpr float StaminaBoostDuration = 15.f;
			constexpr float StaminaBoostMultiplier = 2.0f;
			StatsComp->AddBuff(EBMBuffType::StaminaRegenBoost, StaminaBoostMultiplier, StaminaBoostDuration);
			Effects.Add(TEXT("Stamina Regen x2 (15s)"));
			
			UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::UseItem - %s 添加耐力恢复加速 (%.1fx)，持续 %.1fs"), 
				*ItemID.ToString(), StaminaBoostMultiplier, StaminaBoostDuration);
			bEffectApplied = true;
			break;
		}
		case EBMItemFunction::Invulnerability:
		{
			// 无敌状态：持续5秒
			constexpr float InvulnerabilityDuration = 5.f;
			StatsComp->AddBuff(EBMBuffType::Invulnerability, 1.0f, InvulnerabilityDuration);
			Effects.Add(TEXT("Invulnerable (5s)"));
			
			UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::UseItem - %s 添加无敌状态，持续 %.1fs"), 
				*ItemID.ToString(), InvulnerabilityDuration);
			bEffectApplied = true;
			break;
		}
		case EBMItemFunction::HP:
		{
			// 持续回血：15秒内恢复50%血量
			constexpr float HPRegenDuration = 15.f;
			constexpr float HPRegenPercent = 50.f;
			StatsComp->AddBuff(EBMBuffType::HealthRegenOverTime, HPRegenPercent, HPRegenDuration);
			Effects.Add(TEXT("HP Regen 50% (15s)"));
			
			UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::UseItem - %s 添加持续回血 (%.1f%% over %.1fs)"), 
				*ItemID.ToString(), HPRegenPercent, HPRegenDuration);
			bEffectApplied = true;
			break;
		}
		case EBMItemFunction::None:
		default:
			// 无额外效果
			break;
	}

	if (!bEffectApplied)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::UseItem - %s 没有可应用的效果"), *ItemID.ToString());
		// 仍然允许使用，可能是纯消耗品
	}

	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::UseItem - 使用物品 %s x%d"), *ItemID.ToString(), Count);

	OnItemUsed.Broadcast(ItemID, Count);
	
	// 移除物品并获取剩余数量
	const bool bRemoved = RemoveItem(ItemID, Count);
	if (!bRemoved)
	{
		return false;
	}

	const int32 RemainingCount = GetItemCount(ItemID);

	// 构建通知消息
	FString ItemName = ItemID.ToString();
	ItemName.RemoveFromStart(TEXT("Item_"));

	FString NotificationMessage = FString::Printf(TEXT("Used: %s"), *ItemName);
	
	if (Effects.Num() > 0)
	{
		NotificationMessage += TEXT(" | Effects: ");
		for (int32 i = 0; i < Effects.Num(); ++i)
		{
			NotificationMessage += Effects[i];
			if (i < Effects.Num() - 1)
			{
				NotificationMessage += TEXT(", ");
			}
		}
	}

	NotificationMessage += FString::Printf(TEXT(" | Remaining: %d"), RemainingCount);

	// 通过事件总线发送通知
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UBMEventBusSubsystem* EventBus = GI->GetSubsystem<UBMEventBusSubsystem>())
		{
			EventBus->EmitNotify(FText::FromString(NotificationMessage));
		}
	}

	return true;
}

/*
 * @brief Equip item, it equips the item
 * @param ItemID The item ID
 * @return True if the item is equipped, false otherwise
 */
bool UBMInventoryComponent::EquipItem(FName ItemID)
{
	if (!HasItem(ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::EquipItem - 物品 %s 不存在"), *ItemID.ToString());
		return false;
	}

	const FBMItemData* ItemData = GetItemData(ItemID);
	if (!ItemData)
	{
		return false;
	}

	// TODO: 将装备信息同步到角色（例如武器、护甲组件）
	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::EquipItem - 装备物品 %s"), *ItemID.ToString());

	OnInventoryChanged.Broadcast();
	return true;
}

/*
 * @brief Drop item, it drops the item
 * @param ItemID The item ID
 * @param Count The count
 * @return True if the item is dropped, false otherwise
 */
bool UBMInventoryComponent::DropItem(FName ItemID, int32 Count)
{
	if (Count <= 0)
	{
		return false;
	}

	if (GetItemCount(ItemID) < Count)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::DropItem - 物品 %s 数量不足"), *ItemID.ToString());
		return false;
	}

	// TODO: 在角色位置生成掉落物Actor
	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::DropItem - 丢弃物品 %s x%d"), *ItemID.ToString(), Count);

	return RemoveItem(ItemID, Count);
}

/*
 * @brief Has item, it checks if the item is in the inventory
 * @param ItemID The item ID
 * @return True if the item is in the inventory, false otherwise
 */
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

/*
 * @brief Get item count, it gets the count of the item
 * @param ItemID The item ID
 * @return The count of the item
 */
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

/*
 * @brief Can afford, it checks if the currency can afford the cost
 * @param Cost The cost
 * @return True if the currency can afford the cost, false otherwise
 */
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

/*
 * @brief Add currency, it adds the currency to the inventory
 * @param Amount The amount
 * @return True if the currency is added, false otherwise
 */
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

	OnInventoryChanged.Broadcast();
	return true;
}

/*
 * @brief Spend currency, it spends the currency from the inventory
 * @param Amount The amount
 * @return True if the currency is spent, false otherwise
 */
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
		
		// 通知用户货币不足
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (UBMEventBusSubsystem* EventBus = GI->GetSubsystem<UBMEventBusSubsystem>())
				{
					FString NotificationMessage = FString::Printf(TEXT("Insufficient Currency: Need %d, Have %d"), Amount, Currency);
					EventBus->EmitNotify(FText::FromString(NotificationMessage));
				}
			}
		}
		
		return false;
	}

	// 消费货币
	Currency -= Amount;
	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::SpendCurrency - 消费货币 %d，剩余货币: %d"), Amount, Currency);

	// 通知用户货币消费成功
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UBMEventBusSubsystem* EventBus = GI->GetSubsystem<UBMEventBusSubsystem>())
			{
				FString NotificationMessage = FString::Printf(TEXT("Spent: %d | Remaining Currency: %d"), Amount, Currency);
				EventBus->EmitNotify(FText::FromString(NotificationMessage));
			}
		}
	}

	OnInventoryChanged.Broadcast();
	return true;
}

/*
 * @brief Get item type count, it gets the count of the item type
 * @return The count of the item type
 */
int32 UBMInventoryComponent::GetItemTypeCount() const
{
	// 返回当前背包中不同物品的种类数量
	return Items.Num();
}

/*
 * @brief Get remaining capacity, it gets the remaining capacity
 * @return The remaining capacity
 */
int32 UBMInventoryComponent::GetRemainingCapacity() const
{
	// 计算剩余容量 = 总容量 - 当前物品种类数量
	int32 Remaining = Capacity - GetItemTypeCount();
	return FMath::Max(0, Remaining); // 确保不为负数
}

/*
 * @brief Is full, it checks if the inventory is full
 * @return True if the inventory is full, false otherwise
 */
bool UBMInventoryComponent::IsFull() const
{
	// 检查当前物品种类数量是否达到容量上限
	return GetItemTypeCount() >= Capacity;
}

/*
 * @brief Set capacity, it sets the capacity
 * @param NewCapacity The new capacity
 */
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
	OnInventoryChanged.Broadcast();
}

/*
 * @brief Clear inventory, it clears the inventory
 */
void UBMInventoryComponent::ClearInventory()
{
	// 清空所有物品
	int32 ItemCount = Items.Num();
	if (ItemCount == 0)
	{
		return;
	}

	Items.Empty();
	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::ClearInventory - 已清空背包，移除了 %d 种物品"), ItemCount);
	OnInventoryChanged.Broadcast();
}

// ==================== 物品信息查询方法实现 ====================

/*
 * @brief Get item name, it gets the name of the item
 * @param ItemID The item ID
 * @return The name of the item
 */
FText UBMInventoryComponent::GetItemName(FName ItemID) const
{
	const FBMItemData* ItemData = GetItemData(ItemID);
	if (ItemData)
	{
		return ItemData->Name;
	}
	return FText::GetEmpty();
}

/*
 * @brief Get item icon, it gets the icon of the item
 * @param ItemID The item ID
 * @return The icon of the item
 */
FSoftObjectPath UBMInventoryComponent::GetItemIcon(FName ItemID) const
{
	const FBMItemData* ItemData = GetItemData(ItemID);
	if (ItemData)
	{
		return ItemData->IconPath;
	}
	return FSoftObjectPath();
}

/*
 * @brief Get item price, it gets the price of the item
 * @param ItemID The item ID
 * @return The price of the item
 */
float UBMInventoryComponent::GetItemPrice(FName ItemID) const
{
	if (bTestForceItemPrice)
	{
		return TestForcedItemPrice;
	}

	const FBMItemData* ItemData = GetItemData(ItemID);
	if (ItemData)
	{
		return ItemData->Price;
	}
	return 0.0f;
}

/*
 * @brief Get item max stack, it gets the max stack of the item
 * @param ItemID The item ID
 * @return The max stack of the item
 */
int32 UBMInventoryComponent::GetItemMaxStack(FName ItemID) const
{
	const FBMItemData* ItemData = GetItemData(ItemID);
	if (ItemData)
	{
		return ItemData->MaxStack;
	}
	return 0;
}

/*
 * @brief Can stack more, it checks if the item can stack more
 * @param ItemID The item ID
 * @param AdditionalCount The additional count
 * @return True if the item can stack more, false otherwise
 */
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

/*
 * @brief Get item data, it gets the data of the item
 * @param ItemID The item ID
 * @return The data of the item
 */
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

/*
 * @brief Get data subsystem, it gets the data subsystem
 * @return The data subsystem
 */
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

// ==================== UI 相关方法实现 ====================

/*
 * @brief Get all item IDs, it gets all the item IDs
 * @param OutItemIDs The out item IDs
 */
void UBMInventoryComponent::GetAllItemIDs(TArray<FName>& OutItemIDs) const
{
	Items.GetKeys(OutItemIDs);
}

/*
 * @brief Select item, it selects the item
 * @param ItemID The item ID
 */
void UBMInventoryComponent::SelectItem(FName ItemID)
{
	// 检查物品是否存在
	if (ItemID != NAME_None && !Items.Contains(ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::SelectItem - 物品 %s 不存在于背包中"), *ItemID.ToString());
		return;
	}

	// 更新选中的物品ID
	SelectedItemID = ItemID;

	// 获取物品数量
	int32 Count = GetItemCount(ItemID);

	// 广播物品选中事件
	OnItemSelected.Broadcast(ItemID, Count);

	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::SelectItem - 选中物品 %s，数量: %d"), *ItemID.ToString(), Count);
}

/*
 * @brief Open inventory UI, it opens the inventory UI
 * @return True if the inventory UI is opened, false otherwise
 */
bool UBMInventoryComponent::OpenInventoryUI()
{
	// 检查是否已设置Widget类
	if (!InventoryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::OpenInventoryUI - InventoryWidgetClass未设置"));
		return false;
	}

	// 如果已经打开，直接返回
	if (InventoryWidgetInstance && InventoryWidgetInstance->IsInViewport())
	{
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::OpenInventoryUI - 背包UI已经打开"));
		return true;
	}

	// 获取玩家控制器
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::OpenInventoryUI - 无法获取玩家控制器"));
		return false;
	}

	// 创建Widget实例
	InventoryWidgetInstance = CreateWidget<UUserWidget>(PC, InventoryWidgetClass);
	if (!InventoryWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::OpenInventoryUI - 创建Widget失败"));
		return false;
	}

	if (UUBMInventoryWidget* InventoryWidget = Cast<UUBMInventoryWidget>(InventoryWidgetInstance))
	{
		InventoryWidget->SetInventoryComponent(this);
	}

	// 添加到视口
	InventoryWidgetInstance->AddToViewport(100);

	// 绑定UI事件
	BindUIEvents();

	// 设置输入模式为UI模式，并显示鼠标
	PC->SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// Don't force focus to the widget, otherwise the widget may consume number key events
	// and prevent PlayerInputComponent hotkeys (1-9) from firing.
	PC->SetInputMode(InputMode);

	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::OpenInventoryUI - 背包UI已打开"));
	return true;
}

/*
 * @brief Close inventory UI, it closes the inventory UI
 */
void UBMInventoryComponent::CloseInventoryUI()
{
	if (!InventoryWidgetInstance)
	{
		return;
	}

	// 解绑UI事件
	UnbindUIEvents();

	// 从视口移除
	InventoryWidgetInstance->RemoveFromParent();
	InventoryWidgetInstance = nullptr;

	// 清除选中状态
	SelectedItemID = NAME_None;

	// 恢复输入模式
	APlayerController* PC = GetPlayerController();
	if (PC)
	{
		PC->SetShowMouseCursor(false);
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::CloseInventoryUI - 背包UI已关闭"));
}

/*
 * @brief Toggle inventory UI, it toggles the inventory UI
 */
void UBMInventoryComponent::ToggleInventoryUI()
{
	if (IsInventoryUIVisible())
	{
		CloseInventoryUI();
	}
	else
	{
		OpenInventoryUI();
	}
}

/*
 * @brief Is inventory UI visible, it checks if the inventory UI is visible
 * @return True if the inventory UI is visible, false otherwise
 */
bool UBMInventoryComponent::IsInventoryUIVisible() const
{
	return InventoryWidgetInstance && InventoryWidgetInstance->IsInViewport();
}

/*
 * @brief Refresh inventory UI, it refreshes the inventory UI
 */
void UBMInventoryComponent::RefreshInventoryUI()
{
	// 广播背包变化事件，让UI自行刷新
	OnInventoryChanged.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::RefreshInventoryUI - 已请求UI刷新"));
}

/*
 * @brief Set inventory widget class, it sets the inventory widget class
 * @param InWidgetClass The inventory widget class
 */
void UBMInventoryComponent::SetInventoryWidgetClass(TSubclassOf<UUserWidget> InWidgetClass)
{
	if (IsInventoryUIVisible())
	{
		UE_LOG(LogTemp, Warning, TEXT("UBMInventoryComponent::SetInventoryWidgetClass - Cannot change widget class while UI is visible"));
		return;
	}

	InventoryWidgetClass = InWidgetClass;
}

/*
 * @brief Toggle test auto add currency, it toggles the test auto add currency
 */
void UBMInventoryComponent::ToggleTestAutoAddCurrency()
{
	bTestAutoAddCurrency = !bTestAutoAddCurrency;

	if (bTestAutoAddCurrency)
	{
		StartTestAutoAddCurrency();
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::ToggleTestAutoAddCurrency - Enabled"));
	}
	else
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TestAutoAddCurrencyTimer);
		}
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::ToggleTestAutoAddCurrency - Disabled"));
	}
}

/*
 * @brief Toggle test force item price 10, it toggles the test force item price 10
 */
void UBMInventoryComponent::ToggleTestForceItemPrice10()
{
	bTestForceItemPrice = !bTestForceItemPrice;
	if (bTestForceItemPrice)
	{
		TestForcedItemPrice = 10.0f;
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::ToggleTestForceItemPrice10 - Enabled (price=10)"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::ToggleTestForceItemPrice10 - Disabled"));
	}

	OnInventoryChanged.Broadcast();
}

/*
 * @brief Get player controller, it gets the player controller
 * @return The player controller
 */
APlayerController* UBMInventoryComponent::GetPlayerController() const
{
	// 尝试从Owner获取
	if (AActor* Owner = GetOwner())
	{
		// 如果Owner是Pawn，获取其Controller
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<APlayerController>(Pawn->GetController());
		}
		// 如果Owner是PlayerController
		if (APlayerController* PC = Cast<APlayerController>(Owner))
		{
			return PC;
		}
	}

	// 回退：获取第一个本地玩家控制器
	if (UWorld* World = GetWorld())
	{
		return UGameplayStatics::GetPlayerController(World, 0);
	}

	return nullptr;
}

/*
 * @brief Bind UI events, it binds the UI events
 */
void UBMInventoryComponent::BindUIEvents()
{
	// 这里可以绑定额外的UI事件
	// 例如：当背包变化时自动刷新UI
	// 由于OnInventoryChanged已经是公开的委托，UI可以直接绑定
	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::BindUIEvents - UI事件已绑定"));
}

/*
 * @brief Unbind UI events, it unbinds the UI events
 */
void UBMInventoryComponent::UnbindUIEvents()
{
	// 解绑UI事件
	UE_LOG(LogTemp, Log, TEXT("UBMInventoryComponent::UnbindUIEvents - UI事件已解绑"));
}
