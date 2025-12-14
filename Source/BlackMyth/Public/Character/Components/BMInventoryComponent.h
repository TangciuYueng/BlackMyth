// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BMInventoryComponent.generated.h"

// 前向声明
class UBMDataSubsystem;
struct FBMItemData;

/**
 * 背包组件
 * 负责管理角色的物品存储、添加、移除和货币系统
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent = "false"))
class BLACKMYTH_API UBMInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/**
	 * 构造函数
	 * 初始化默认值
	 */
	UBMInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/**
	 * 游戏开始时调用
	 */
	virtual void BeginPlay() override;

	/**
	 * 背包容量
	 * 限制玩家能携带的不同物品种类数量上限（不是总物品数量）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 Capacity = 20;

	/**
	 * 物品存储容器
	 * Key: 物品的唯一标识符（ItemID）
	 * Value: 该物品的数量（堆叠数）
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TMap<FName, int32> Items;

	/**
	 * 货币/金币
	 * 存储当前持有的金钱数量，用于交易系统
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "0"))
	int32 Currency = 0;

public:	
	/**
	 * 向背包添加物品
	 * @param ItemID 物品的唯一标识符
	 * @param Count 要添加的数量（必须大于0）
	 * @return 如果添加成功返回true，否则返回false
	 * 
	 * 逻辑说明：
	 * - 检查ItemID是否有效
	 * - 检查Count是否为正数
	 * - 验证ItemID是否在数据表中存在
	 * - 如果物品已存在，检查堆叠限制并增加数量
	 * - 如果物品不存在，检查容量是否已满
	 * - 如果容量未满，创建新条目并添加
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemID, int32 Count);

	/**
	 * 从背包移除物品
	 * @param ItemID 物品的唯一标识符
	 * @param Count 要移除的数量（必须大于0）
	 * @return 如果移除成功返回true，否则返回false
	 * 
	 * 逻辑说明：
	 * - 检查ItemID是否有效
	 * - 检查Count是否为正数
	 * - 检查物品是否存在
	 * - 检查数量是否足够
	 * - 如果数量归零，从Map中移除该Key
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemID, int32 Count);

	/**
	 * 检查背包中是否拥有某物品
	 * @param ItemID 物品的唯一标识符
	 * @return 如果拥有该物品返回true，否则返回false
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItem(FName ItemID) const;

	/**
	 * 查询某物品的具体数量
	 * @param ItemID 物品的唯一标识符
	 * @return 返回指定ItemID对应的数量。如果不拥有该物品，返回0
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;

	/**
	 * 检查是否买得起某样东西
	 * @param Cost 需要花费的金额
	 * @return 如果货币足够返回true，否则返回false
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanAfford(int32 Cost) const;

	/**
	 * 添加货币
	 * @param Amount 要添加的金额（必须大于0）
	 * @return 如果添加成功返回true，否则返回false
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddCurrency(int32 Amount);

	/**
	 * 消费货币
	 * @param Amount 要消费的金额（必须大于0）
	 * @return 如果消费成功返回true，否则返回false（货币不足时返回false）
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SpendCurrency(int32 Amount);

	/**
	 * 获取当前背包中不同物品的种类数量
	 * @return 当前物品种类数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemTypeCount() const;

	/**
	 * 获取背包剩余容量
	 * @return 剩余可容纳的物品种类数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetRemainingCapacity() const;

	/**
	 * 检查背包是否已满
	 * @return 如果已满返回true，否则返回false
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsFull() const;

	/**
	 * 获取当前货币数量
	 * @return 当前货币数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetCurrency() const { return Currency; }

	/**
	 * 获取背包容量
	 * @return 背包容量
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetCapacity() const { return Capacity; }

	/**
	 * 设置背包容量
	 * @param NewCapacity 新的容量值（必须大于0）
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCapacity(int32 NewCapacity);

	/**
	 * 清空背包（移除所有物品）
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearInventory();

	// ==================== 物品信息查询方法 ====================
	
	/**
	 * 获取物品名称
	 * @param ItemID 物品的唯一标识符
	 * @return 物品名称，如果物品不存在则返回空文本
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FText GetItemName(FName ItemID) const;

	/**
	 * 获取物品图标路径
	 * @param ItemID 物品的唯一标识符
	 * @return 物品图标路径，如果物品不存在则返回空路径
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FSoftObjectPath GetItemIcon(FName ItemID) const;

	/**
	 * 获取物品价格
	 * @param ItemID 物品的唯一标识符
	 * @return 物品价格，如果物品不存在则返回0.0f
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetItemPrice(FName ItemID) const;

	/**
	 * 获取物品最大堆叠数
	 * @param ItemID 物品的唯一标识符
	 * @return 物品最大堆叠数，如果物品不存在则返回0
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemMaxStack(FName ItemID) const;

	/**
	 * 检查物品是否可以继续堆叠更多数量
	 * @param ItemID 物品的唯一标识符
	 * @param AdditionalCount 要添加的数量
	 * @return 如果可以堆叠返回true，否则返回false
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanStackMore(FName ItemID, int32 AdditionalCount) const;

	/**
	 * 获取物品数据
	 * @param ItemID 物品的唯一标识符
	 * @return 物品数据指针，如果物品不存在则返回nullptr
	 */
	const FBMItemData* GetItemData(FName ItemID) const;

private:
	/**
	 * 获取数据子系统（缓存以提高性能）
	 * @return 数据子系统指针
	 */
	UBMDataSubsystem* GetDataSubsystem() const;
};
