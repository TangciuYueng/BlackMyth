// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BMShopComponent.generated.h"

class UBMDataSubsystem;
struct FBMItemData;

/**
 * 商店状态发生变化（金币、库存、仓库等）时广播给UI的事件
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopChanged);

USTRUCT(BlueprintType)
struct FBMShopSlot
{
	GENERATED_BODY()

public:
	/** ItemID = NAME_None 表示该槽位无效，不会出现在货架上 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FName ItemID = NAME_None;

	/** PriceOverride < 0 则采用数据表价格，>=0 使用自定义价格（仅用于显示或与玩家交易） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	float PriceOverride = -1.f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBMShopComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** UI 层绑定此事件即可在仓库变化时实时刷新 */
	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnShopChanged OnShopChanged;

	/**
	 * 购买指定物品（无限量供给），只会写入商店仓库
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool PurchaseItem(FName ItemID, int32 Count = 1);

	/** 查询仓库存放的物品数量 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	int32 GetWarehouseCount(FName ItemID) const;

	/** 拷贝当前货架信息用于UI展示 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	TArray<FBMShopSlot> GetShelfSnapshot() const;

	/** 直接访问仓库数据的快照 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	const TMap<FName, int32>& GetWarehouseSnapshot() const { return WarehouseItems; }

protected:
	virtual void BeginPlay() override;

	/** 默认配置的货架（仅描述商品，不记录数量） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TArray<FBMShopSlot> DefaultShelf;

	/** 商店的仓库存货，购买完成后物品会进入这里 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
	TMap<FName, int32> WarehouseItems;

private:
	/** 可供购买的物品ID集合，BeginPlay时由 DefaultShelf 构建 */
	TSet<FName> AvailableItems;

	/** 重建可售列表，通常用于初始化 */
	void RebuildRuntimeShelf();
	/** 从数据子系统读取物品配置 */
	const FBMItemData* GetItemData(FName ItemID) const;
	/** 在 DefaultShelf 中寻找对应的槽位定义 */
	const FBMShopSlot* FindSlotDefinition(FName ItemID) const;
	/** 计算最终用于展示/结算的单价 */
	float ResolvePrice(const FBMItemData& ItemData, const FBMShopSlot* SlotDefinition) const;
	/** 通知UI刷新 */
	void BroadcastShopChanged();
	/** 缓存获取数据子系统的方法 */
	UBMDataSubsystem* GetDataSubsystem() const;
};
