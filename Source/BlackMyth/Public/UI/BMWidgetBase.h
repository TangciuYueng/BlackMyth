// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UBMEventBusSubsystem;
class UBMUIManagerSubsystem;
class UTextBlock;
class UBMDataSubsystem;

#include "BMWidgetBase.generated.h"

/**
 * @brief Define the UBMWidgetBase class
 * @param UBMWidgetBase The name of the class
 * @param UUserWidget The parent class
 */
UCLASS()
class BLACKMYTH_API UBMWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    // Bind/unbind to the EventBus. Derived widgets override to subscribe their own handlers.
    virtual void BindEventBus(UBMEventBusSubsystem* EventBus);
    virtual void UnbindEventBus(UBMEventBusSubsystem* EventBus);

    UBMEventBusSubsystem* GetEventBus() const;
    UBMUIManagerSubsystem* GetUIManager() const;

    // Auto-fill item prices into TextBlocks named with a convention:
    //   - Price_<ItemID>
    //   - ItemPrice_<ItemID>
    // Example: a TextBlock named Price_Item_JiuZhuanJinDan will show that item's price
    UFUNCTION(BlueprintCallable, Category = "BM|UI|ItemPrice")
    void RefreshNamedItemPrices();

private:
    UBMEventBusSubsystem* CachedEventBus = nullptr;
};
