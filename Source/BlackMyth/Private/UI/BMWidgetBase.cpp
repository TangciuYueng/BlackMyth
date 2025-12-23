// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMWidgetBase.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Character/Components/BMInventoryComponent.h"
#include "Core/BMDataSubsystem.h"

void UBMWidgetBase::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

void UBMWidgetBase::NativeConstruct()
{
    Super::NativeConstruct();

    if (UGameInstance* GI = GetGameInstance())
    {
        CachedEventBus = GI->GetSubsystem<UBMEventBusSubsystem>();
        if (CachedEventBus)
        {
            BindEventBus(CachedEventBus);
        }
    }

    // On construct, attempt to populate any price TextBlocks
    RefreshNamedItemPrices();
}

void UBMWidgetBase::NativeDestruct()
{
    if (CachedEventBus)
    {
        UnbindEventBus(CachedEventBus);
        CachedEventBus = nullptr;
    }

    Super::NativeDestruct();
}

void UBMWidgetBase::BindEventBus(UBMEventBusSubsystem* /*EventBus*/)
{
}

void UBMWidgetBase::UnbindEventBus(UBMEventBusSubsystem* /*EventBus*/)
{
}

UBMEventBusSubsystem* UBMWidgetBase::GetEventBus() const
{
    return CachedEventBus;
}

UBMUIManagerSubsystem* UBMWidgetBase::GetUIManager() const
{
    if (UGameInstance* GI = GetGameInstance())
    {
        return GI->GetSubsystem<UBMUIManagerSubsystem>();
    }
    return nullptr;
}

void UBMWidgetBase::RefreshNamedItemPrices()
{
    if (!WidgetTree) return;

    // Helper to resolve price by ItemID
    auto ResolvePrice = [this](const FName& ItemID) -> float
    {
        // Prefer player inventory (may apply testing overrides)
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (APawn* Pawn = PC->GetPawn())
            {
                if (UBMInventoryComponent* Inv = Pawn->FindComponentByClass<UBMInventoryComponent>())
                {
                    return Inv->GetItemPrice(ItemID);
                }
            }
        }

        // Fallback to data subsystem
        if (UGameInstance* GI = GetGameInstance())
        {
            if (const UBMDataSubsystem* Data = GI->GetSubsystem<UBMDataSubsystem>())
            {
                if (const FBMItemData* Row = Data->GetItemData(ItemID))
                {
                    return Row->Price;
                }
            }
        }
        return 0.f;
    };

    TArray<UWidget*> AllWidgets;
    WidgetTree->GetAllWidgets(AllWidgets);
    for (UWidget* W : AllWidgets)
    {
        UTextBlock* TB = Cast<UTextBlock>(W);
        if (!TB) continue;

        const FString Name = TB->GetName();
        FString ItemIdStr;
        // Single supported pattern: Price_<ItemID>
        const FString Prefix = TEXT("Price_");
        if (Name.StartsWith(Prefix))
        {
            ItemIdStr = Name.RightChop(Prefix.Len());
        }
        else
        {
            continue;
        }

        if (ItemIdStr.IsEmpty()) continue;
        const FName ItemID(*ItemIdStr);
        const float Price = ResolvePrice(ItemID);
        const FString PriceStr = FString::SanitizeFloat(Price);
        TB->SetText(FText::FromString(PriceStr));
    }
}

