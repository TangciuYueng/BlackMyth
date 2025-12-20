// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UBMInventoryWidget.h"

#include "Character/Components/BMInventoryComponent.h"

#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"

const FName UUBMInventoryWidget::HotbarItemIDs[9] = {
	TEXT("Item_JiuZhuanJinDan"),
	TEXT("Item_TaiYiZiJinDan"),
	TEXT("Item_CheDianWei"),
	TEXT("Item_TieShiXin"),
	TEXT("Item_ChuBaiQiangTou"),
	TEXT("Item_JinChenXin"),
	TEXT("Item_JinGuangYanMou"),
	TEXT("Item_YinXingWuJiao"),
	TEXT("Item_YaoShengJiao")
};

int32 UUBMInventoryWidget::GetItemCountForUI(FName ItemID) const
{
	if (const UBMInventoryComponent* Inv = InventoryComponent.Get())
	{
		return Inv->GetItemCount(ItemID);
	}
	return 0;
}

void UUBMInventoryWidget::GetAllItemIDsForUI(TArray<FName>& OutItemIDs) const
{
	OutItemIDs.Reset();
	if (const UBMInventoryComponent* Inv = InventoryComponent.Get())
	{
		Inv->GetAllItemIDs(OutItemIDs);
	}
}

bool UUBMInventoryWidget::UseItemForUI(FName ItemID, int32 Count)
{
	UBMInventoryComponent* Inv = InventoryComponent.Get();
	if (!Inv)
	{
		return false;
	}

	return Inv->UseItem(ItemID, Count);
}

bool UUBMInventoryWidget::BuyItemForUI(FName ItemID, int32 Count)
{
	UBMInventoryComponent* Inv = InventoryComponent.Get();
	if (!Inv)
	{
		return false;
	}

	if (Count <= 0)
	{
		return false;
	}

	const float UnitPrice = Inv->GetItemPrice(ItemID);
	const int32 UnitCost = FMath::Max(0, FMath::RoundToInt(UnitPrice));
	const int32 TotalCost = UnitCost * Count;

	if (TotalCost > 0 && !Inv->SpendCurrency(TotalCost))
	{
		return false;
	}

	return Inv->AddItem(ItemID, Count);
}

FName UUBMInventoryWidget::GetHotbarItemID(int32 SlotIndex) const
{
	if (SlotIndex < 1 || SlotIndex > 9)
	{
		return NAME_None;
	}
	return HotbarItemIDs[SlotIndex - 1];
}

bool UUBMInventoryWidget::TriggerHotbarSlot(int32 SlotIndex, int32 Count)
{
	UBMInventoryComponent* Inv = InventoryComponent.Get();
	if (!Inv)
	{
		return false;
	}

	const FName ItemID = GetHotbarItemID(SlotIndex);
	if (ItemID == NAME_None)
	{
		return false;
	}

	// When UI is visible, treat the hotbar key as "buy"; otherwise treat as "use".
	if (Inv->IsInventoryUIVisible())
	{
		return BuyItemForUI(ItemID, Count);
	}

	return UseItemForUI(ItemID, Count);
}

void UUBMInventoryWidget::SetInventoryComponent(UBMInventoryComponent* InInventory)
{
	if (InventoryComponent.Get() == InInventory)
	{
		return;
	}

	if (UBMInventoryComponent* Old = InventoryComponent.Get())
	{
		Old->OnInventoryChanged.RemoveAll(this);
	}

	InventoryComponent = InInventory;

	if (UBMInventoryComponent* NewInv = InventoryComponent.Get())
	{
		NewInv->OnInventoryChanged.AddDynamic(this, &UUBMInventoryWidget::HandleInventoryChanged);
	}

	RebuildItemList();
}

void UUBMInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RebuildItemList();
}

void UUBMInventoryWidget::NativeDestruct()
{
	if (UBMInventoryComponent* Inv = InventoryComponent.Get())
	{
		Inv->OnInventoryChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UUBMInventoryWidget::HandleInventoryChanged()
{
	RebuildItemList();
	BP_OnInventoryChanged();
}

void UUBMInventoryWidget::RebuildItemList()
{
	UBMInventoryComponent* Inv = InventoryComponent.Get();

	if (JiuZhuanJinDanText)
	{
		JiuZhuanJinDanText->SetText(FText::AsNumber(Inv ? Inv->GetItemCount(TEXT("Item_JiuZhuanJinDan")) : 0));
	}
	if (TaiYiZiJinDanText)
	{
		TaiYiZiJinDanText->SetText(FText::AsNumber(Inv ? Inv->GetItemCount(TEXT("Item_TaiYiZiJinDan")) : 0));
	}
	if (CheDianWeiText)
	{
		CheDianWeiText->SetText(FText::AsNumber(Inv ? Inv->GetItemCount(TEXT("Item_CheDianWei")) : 0));
	}
	if (TieShiXinText)
	{
		TieShiXinText->SetText(FText::AsNumber(Inv ? Inv->GetItemCount(TEXT("Item_TieShiXin")) : 0));
	}
	if (ChuBaiQiangTouText)
	{
		ChuBaiQiangTouText->SetText(FText::AsNumber(Inv ? Inv->GetItemCount(TEXT("Item_ChuBaiQiangTou")) : 0));
	}
	if (JinChenXinText)
	{
		JinChenXinText->SetText(FText::AsNumber(Inv ? Inv->GetItemCount(TEXT("Item_JinChenXin")) : 0));
	}
	if (JinGuangYanMouText)
	{
		JinGuangYanMouText->SetText(FText::AsNumber(Inv ? Inv->GetItemCount(TEXT("Item_JinGuangYanMou")) : 0));
	}
	if (YinXingWuJiaoText)
	{
		YinXingWuJiaoText->SetText(FText::AsNumber(Inv ? Inv->GetItemCount(TEXT("Item_YinXingWuJiao")) : 0));
	}
	if (YaoShengJiaoText)
	{
		YaoShengJiaoText->SetText(FText::AsNumber(Inv ? Inv->GetItemCount(TEXT("Item_YaoShengJiao")) : 0));
	}

	if (CurrencyText)
	{
		CurrencyText->SetText(FText::FromString(Inv ? FString::Printf(TEXT("%d"), Inv->GetCurrency()) : TEXT("0")));
	}

	if (!ItemList)
	{
		return;
	}

	ItemList->ClearChildren();

	if (!Inv)
	{
		UTextBlock* EmptyText = NewObject<UTextBlock>(this);
		EmptyText->SetText(FText::FromString(TEXT("No Inventory")));
		ItemList->AddChild(EmptyText);
		return;
	}

	TArray<FName> ItemIds;
	Inv->GetAllItemIDs(ItemIds);
	ItemIds.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });

	if (ItemIds.Num() == 0)
	{
		UTextBlock* EmptyText = NewObject<UTextBlock>(this);
		EmptyText->SetText(FText::FromString(TEXT("Empty")));
		ItemList->AddChild(EmptyText);
		return;
	}

	for (const FName& ItemId : ItemIds)
	{
		const int32 Count = Inv->GetItemCount(ItemId);
		const FText Name = Inv->GetItemName(ItemId);
		const FString Line = FString::Printf(TEXT("%s x%d"), Name.IsEmpty() ? *ItemId.ToString() : *Name.ToString(), Count);

		UTextBlock* RowText = NewObject<UTextBlock>(this);
		RowText->SetText(FText::FromString(Line));
		ItemList->AddChild(RowText);
	}
}

