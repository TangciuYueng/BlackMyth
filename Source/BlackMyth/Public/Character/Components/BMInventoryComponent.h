// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blueprint/UserWidget.h"
#include "BMInventoryComponent.generated.h"

/**
 * 动态多播委托，用于在背包内容或货币发生变化时通知UI立刻刷新
 * 这样玩家在游戏内进行操作后，界面层可以第一时间展示数量变化或属性提升
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/**
 * 当物品被选中时触发的委托
 * @param ItemID 被选中的物品ID
 * @param Count 被选中物品的数量
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemSelected, FName, ItemID, int32, Count);

/**
 * 当物品被使用时触发的委托
 * @param ItemID 使用的物品ID
 * @param Count 使用的数量
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemUsed, FName, ItemID, int32, Count);

// 前向声明
class UUserWidget;
class UVerticalBox;
class UScrollBox;
class UTextBlock;
class UButton;
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

	/** 背包内容变化时触发，UI可以绑定该事件实现即时反馈 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryChanged OnInventoryChanged;

	/** 物品被选中时触发，UI可以绑定此事件显示物品详情 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryItemSelected OnItemSelected;

	/** 物品被使用时触发，Gameplay/UI 都可以绑定此事件做属性变更或表现 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryItemUsed OnItemUsed;

	// ==================== UI 相关方法 ====================

	/**
	 * 获取所有物品列表（用于UI遍历显示）
	 * @return 返回物品ID到数量的映射
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	TMap<FName, int32> GetAllItems() const { return Items; }

	/**
	 * 获取所有物品ID数组（用于UI遍历）
	 * @param OutItemIDs 输出的物品ID数组
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void GetAllItemIDs(TArray<FName>& OutItemIDs) const;

	/**
	 * 选中某个物品（触发OnItemSelected委托）
	 * @param ItemID 要选中的物品ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void SelectItem(FName ItemID);

	/**
	 * 获取当前选中的物品ID
	 * @return 当前选中的物品ID，如果没有选中则返回NAME_None
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	FName GetSelectedItemID() const { return SelectedItemID; }

	/**
	 * 打开背包UI
	 * @return 如果成功打开返回true
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	bool OpenInventoryUI();

	/**
	 * 关闭背包UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void CloseInventoryUI();

	/**
	 * 切换背包UI显示状态
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void ToggleInventoryUI();

	/**
	 * 检查背包UI是否正在显示
	 * @return 如果正在显示返回true
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	bool IsInventoryUIVisible() const;

	/**
	 * 刷新背包UI显示
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void RefreshInventoryUI();

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void SetInventoryWidgetClass(TSubclassOf<UUserWidget> InWidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Test")
	void ToggleTestAutoAddCurrency();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Test")
	bool IsTestAutoAddCurrencyEnabled() const { return bTestAutoAddCurrency; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Test")
	void ToggleTestForceItemPrice10();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Test")
	bool IsTestForceItemPriceEnabled() const { return bTestForceItemPrice; }

protected:
	/**
	 * 游戏开始时调用
	 */
	virtual void BeginPlay() override;

	// ==================== 临时测试功能 ====================
	// 每秒自动增加金币（用于UI/逻辑测试，后续可移除或关闭）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Test")
	bool bTestAutoAddCurrency = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Test", meta = (ClampMin = "0.01"))
	float TestAutoAddCurrencyIntervalSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Test", meta = (ClampMin = "1"))
	int32 TestAutoAddCurrencyAmount = 100;

	// 临时：强制所有道具价格（用于购买逻辑测试，后续可关闭或移除）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Test")
	bool bTestForceItemPrice = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Test", meta = (ClampMin = "0"))
	float TestForcedItemPrice = 10.0f;

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

	/**
	 * 背包UI Widget类
	 * 在编辑器中指定要使用的Widget蓝图类
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI")
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	/**
	 * 当前背包UI Widget实例
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|UI")
	UUserWidget* InventoryWidgetInstance = nullptr;

	/**
	 * 当前选中的物品ID
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|UI")
	FName SelectedItemID = NAME_None;

	/**
	 * 背包每行显示的格子数量
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|UI", meta = (ClampMin = "1", ClampMax = "10"))
	int32 SlotsPerRow = 5;

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
	 * 使用物品（例如药品），用于在消耗后立即刷新UI与角色属性
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(FName ItemID, int32 Count = 1);

	/**
	 * 装备物品（例如武器/护甲），让角色与UI同步展示装备状态
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool EquipItem(FName ItemID);

	/**
	 * 丢弃/生成掉落物，物品离开背包时通知UI减少数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool DropItem(FName ItemID, int32 Count = 1);

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

	// ==================== 存档系统接口 ====================

	/**
	 * 获取所有物品（用于存档系统）
	 * @return 所有物品的只读引用
	 */
	const TMap<FName, int32>& GetAllItemsRef() const { return Items; }

	/**
	 * 设置货币（用于存档系统）
	 * 直接设置货币值，不进行验证（存档系统专用）
	 * @param NewCurrency 新的货币值
	 */
	void SetCurrencyDirect(int32 NewCurrency) { Currency = NewCurrency; }

private:
	/**
	 * 获取数据子系统（缓存以提高性能）
	 * @return 数据子系统指针
	 */
	UBMDataSubsystem* GetDataSubsystem() const;

	/**
	 * 获取玩家控制器
	 * @return 玩家控制器指针
	 */
	APlayerController* GetPlayerController() const;

	/**
	 * 绑定UI事件
	 */
	void BindUIEvents();

	/**
	 * 解绑UI事件
	 */
	void UnbindUIEvents();

	void StartTestAutoAddCurrency();

	UFUNCTION()
	void HandleTestAutoAddCurrencyTick();

	FTimerHandle TestAutoAddCurrencyTimer;
};
