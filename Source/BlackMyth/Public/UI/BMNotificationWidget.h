#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "BMNotificationWidget.generated.h"

/**
 * @brief Define the UBMNotificationWidget class
 * @param UBMNotificationWidget The name of the class
 * @param UBMWidgetBase The parent class
 */
UCLASS()
class BLACKMYTH_API UBMNotificationWidget : public UBMWidgetBase
{
    GENERATED_BODY()

public:
    // Message list binding
    UPROPERTY(meta=(BindWidget)) UVerticalBox* MessageList = nullptr;

    // Display a notification with the given text
    UFUNCTION(BlueprintCallable, Category = "Notification")
    void ShowNotification(const FString& Message);

protected:
    // Bind event bus
    virtual void BindEventBus(class UBMEventBusSubsystem* EventBus) override;
    // Unbind event bus
    virtual void UnbindEventBus(class UBMEventBusSubsystem* EventBus) override;
    // Native tick
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    // Notify handle
    FDelegateHandle NotifyHandle;
    // Handle notify
    void HandleNotify(const FText& Msg);

    // Active notifications created at runtime (parallel arrays)
    TArray<TWeakObjectPtr<UTextBlock>> ActiveTextWidgets;
    // Active time remaining
    TArray<float> ActiveTimeRemaining;
    // Active opacity
    TArray<float> ActiveOpacity;

    // Seconds before fade begins
    float InitialDelay = 5.f;
    // Opacity fade speed (opacity per second)
    float FadeSpeed = 1.f;
};
