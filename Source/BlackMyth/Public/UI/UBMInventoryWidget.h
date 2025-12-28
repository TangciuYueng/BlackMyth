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

/**
 * @brief Define the UBMInventoryWidget class, inventory widget, used to display the inventory of the player
 * @param UBMInventoryWidget The name of the class
 * @param UBMWidgetBase The parent class
 */
UCLASS()
class BLACKMYTH_API UUBMInventoryWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
	// Set the inventory component
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void SetInventoryComponent(UBMInventoryComponent* InInventory);

	// Get the inventory component
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	UBMInventoryComponent* GetInventoryComponent() const { return InventoryComponent.Get(); }

	// Get the item count for the UI
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	int32 GetItemCountForUI(FName ItemID) const;

	// Get all item IDs for the UI
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void GetAllItemIDsForUI(TArray<FName>& OutItemIDs) const;

	// Use the item for the UI
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	bool UseItemForUI(FName ItemID, int32 Count = 1);

	// Buy the item for the UI
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	bool BuyItemForUI(FName ItemID, int32 Count = 1);

	// Get the hotbar item ID for the UI
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	FName GetHotbarItemID(int32 SlotIndex) const;

	// Trigger the hotbar slot for the UI
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	bool TriggerHotbarSlot(int32 SlotIndex, int32 Count = 1);

protected:
	// Native construct
	virtual void NativeConstruct() override;
	// Native destruct
	virtual void NativeDestruct() override;

	// On inventory changed
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|UI")
	void BP_OnInventoryChanged();

	// Handle inventory changed
	UFUNCTION()
	void HandleInventoryChanged();

	// Rebuild the item list
	void RebuildItemList();

	// bind these in a Widget Blueprint that uses this class.
	UPROPERTY(meta = (BindWidgetOptional))
	UScrollBox* ItemList = nullptr;

	// Currency text binding
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* CurrencyText = nullptr;

	// Fixed-slot item count texts. In WBP, create TextBlocks with these exact names.
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
	// Inventory component for the widget
	TWeakObjectPtr<UBMInventoryComponent> InventoryComponent;
	// Inventory changed handle
	FDelegateHandle InventoryChangedHandle;

	// Hotbar item IDs
	static const FName HotbarItemIDs[9];
};
