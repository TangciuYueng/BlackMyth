// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "BMNotificationWidget.generated.h"

/**
 * Notification list widget. Blueprint should subclass this and bind a VerticalBox named MessageList.
 */
UCLASS()
class BLACKMYTH_API UBMNotificationWidget : public UBMWidgetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(meta=(BindWidget)) UVerticalBox* MessageList = nullptr;

protected:
    virtual void BindEventBus(class UBMEventBusSubsystem* EventBus) override;
    virtual void UnbindEventBus(class UBMEventBusSubsystem* EventBus) override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    FDelegateHandle NotifyHandle;
    void HandleNotify(const FText& Msg);

    // Active notifications created at runtime (parallel arrays)
    TArray<TWeakObjectPtr<UTextBlock>> ActiveTextWidgets;
    TArray<float> ActiveTimeRemaining;
    TArray<float> ActiveOpacity;

    // Seconds before fade begins
    float InitialDelay = 5.f;
    // Opacity fade speed (opacity per second)
    float FadeSpeed = 1.f;
};
