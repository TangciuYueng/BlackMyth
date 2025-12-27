#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "BMNotificationWidget.generated.h"

/**
 * Notification list widget. 
 */
UCLASS()
class BLACKMYTH_API UBMNotificationWidget : public UBMWidgetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(meta=(BindWidget)) UVerticalBox* MessageList = nullptr;

    // Display a notification with the given text
    UFUNCTION(BlueprintCallable, Category = "Notification")
    void ShowNotification(const FString& Message);

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
