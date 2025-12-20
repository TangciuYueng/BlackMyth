// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "BMWidgetBase.h"
#include "UBMInventoryWidget.generated.h"

class UBMInventoryComponent;
class UScrollBox;
class UTextBlock;

UCLASS()
class BLACKMYTH_API UUBMInventoryWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void SetInventoryComponent(UBMInventoryComponent* InInventory);

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	UBMInventoryComponent* GetInventoryComponent() const { return InventoryComponent.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	int32 GetItemCountForUI(FName ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void GetAllItemIDsForUI(TArray<FName>& OutItemIDs) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	bool UseItemForUI(FName ItemID, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	bool BuyItemForUI(FName ItemID, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	FName GetHotbarItemID(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	bool TriggerHotbarSlot(int32 SlotIndex, int32 Count = 1);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|UI")
	void BP_OnInventoryChanged();

	UFUNCTION()
	void HandleInventoryChanged();

	void RebuildItemList();

	// Optional: bind these in a Widget Blueprint that uses this class.
	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* ItemList = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* CurrencyText = nullptr;

	// Optional fixed-slot item count texts. In WBP, create TextBlocks with these exact names.
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* JiuZhuanJinDanText = nullptr;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TaiYiZiJinDanText = nullptr;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* CheDianWeiText = nullptr;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TieShiXinText = nullptr;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ChuBaiQiangTouText = nullptr;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* JinChenXinText = nullptr;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* JinGuangYanMouText = nullptr;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* YinXingWuJiaoText = nullptr;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* YaoShengJiaoText = nullptr;

private:
	TWeakObjectPtr<UBMInventoryComponent> InventoryComponent;
	FDelegateHandle InventoryChangedHandle;

	static const FName HotbarItemIDs[9];
};
