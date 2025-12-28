// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMWidgetBase.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Character/Components/BMInventoryComponent.h"
#include "Core/BMDataSubsystem.h"

/*
 * @brief Native on initialized, it native on initialized
 */
void UBMWidgetBase::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

/*
 * @brief Native construct, it native construct
 */
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

/*
 * @brief Native destruct, it native destruct
 */
void UBMWidgetBase::NativeDestruct()
{
    if (CachedEventBus)
    {
        UnbindEventBus(CachedEventBus);
        CachedEventBus = nullptr;
    }

    Super::NativeDestruct();
}

/*
 * @brief Bind event bus, it bind event bus
 * @param EventBus The event bus
 */
void UBMWidgetBase::BindEventBus(UBMEventBusSubsystem* /*EventBus*/)
{
}

/*
 * @brief Unbind event bus, it unbind event bus
 * @param EventBus The event bus
 */
void UBMWidgetBase::UnbindEventBus(UBMEventBusSubsystem* /*EventBus*/)
{
}

/*
 * @brief Get event bus, it get event bus
 * @return The event bus
 */
UBMEventBusSubsystem* UBMWidgetBase::GetEventBus() const
{
    return CachedEventBus;
}

/*
 * @brief Get ui manager, it get ui manager
 * @return The ui manager
 */
UBMUIManagerSubsystem* UBMWidgetBase::GetUIManager() const
{
    if (UGameInstance* GI = GetGameInstance())
    {
        return GI->GetSubsystem<UBMUIManagerSubsystem>();
    }
    return nullptr;
}

/*
 * @brief Refresh named item prices, it refresh named item prices
 */
void UBMWidgetBase::RefreshNamedItemPrices()
{
    if (!WidgetTree) return;

    // Helper to resolve price by ItemID
    /*
     * @brief Resolve price, it resolve price
     * @param ItemID The item id
     * @return The price
     */
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

    /*
     * @brief Refresh named item prices, it refresh named item prices
     */
    TArray<UWidget*> AllWidgets;
    WidgetTree->GetAllWidgets(AllWidgets);
    /*
     * @brief Refresh named item prices, it refresh named item prices
     */
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

