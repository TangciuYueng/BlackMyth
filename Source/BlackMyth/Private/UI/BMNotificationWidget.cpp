// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMNotificationWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "Blueprint/WidgetTree.h"


/*
 * @brief Bind event bus, it bind event bus
 * @param EventBus The event bus
 */
void UBMNotificationWidget::BindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;
    if (!NotifyHandle.IsValid())
    {
        NotifyHandle = EventBus->OnNotifyMessage.AddWeakLambda(this, [this](const FText& Msg)
        {
            HandleNotify(Msg);
        });
    }
}

/*
 * @brief Unbind event bus, it unbind event bus
 * @param EventBus The event bus
 */
void UBMNotificationWidget::UnbindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;
    if (NotifyHandle.IsValid())
    {
        EventBus->OnNotifyMessage.Remove(NotifyHandle);
        NotifyHandle.Reset();
    }
}

/*
 * @brief Show notification, it show notification
 * @param Message The message
 */
void UBMNotificationWidget::ShowNotification(const FString& Message)
{
    if (Message.IsEmpty()) return;
    HandleNotify(FText::FromString(Message));
}

/*
 * @brief Handle notify, it handle notify
 * @param Msg The message
 */
void UBMNotificationWidget::HandleNotify(const FText& Msg)
{
    if (!MessageList) return;
    // Create a text block via WidgetTree so it is properly initialized for UMG
    UTextBlock* Text = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : NewObject<UTextBlock>(this);
    if (!Text) return;
    Text->SetText(Msg);
    Text->SetOpacity(1.f);
    MessageList->AddChildToVerticalBox(Text);

    // Track it for delayed fade-out
    ActiveTextWidgets.Add(Text);
    ActiveTimeRemaining.Add(InitialDelay);
    ActiveOpacity.Add(1.f);
}

/*
 * @brief Native tick, it native tick
 * @param MyGeometry The geometry
 * @param InDeltaTime The delta time
 */
void UBMNotificationWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // If there are no active text widgets, return
    if (ActiveTextWidgets.Num() == 0) return;

    // Loop through the active text widgets and handle the notifications
    for (int32 i = ActiveTextWidgets.Num() - 1; i >= 0; --i)
    {
        // Get the text block pointer
        TWeakObjectPtr<UTextBlock>& TextPtr = ActiveTextWidgets[i];
        if (!TextPtr.IsValid())
        {
            ActiveTextWidgets.RemoveAt(i);
            ActiveTimeRemaining.RemoveAt(i);
            ActiveOpacity.RemoveAt(i);
            continue;
        }

        float& TimeRem = ActiveTimeRemaining[i];
        float& Op = ActiveOpacity[i];
        if (TimeRem > 0.f)
        {
            TimeRem -= InDeltaTime;
            continue;
        }

        // Update the opacity
        Op = FMath::Max(0.f, Op - FadeSpeed * InDeltaTime);
        // Get the text block pointer
        if (UTextBlock* TB = TextPtr.Get())
        {
            // Get the color and opacity
            FSlateColor Color = TB->ColorAndOpacity;
            FLinearColor Linear = Color.GetSpecifiedColor();
            Linear.A = Op;
            // Set the color and opacity
            TB->SetColorAndOpacity(FSlateColor(Linear));
        }

        // If the opacity is less than a small number, remove the text block
        if (Op <= KINDA_SMALL_NUMBER)
        {
            // If the text block is valid, remove it from the message list
            if (UWidget* W = TextPtr.Get())
            {
                MessageList->RemoveChild(W);
            }
            // Remove the text block from the active text widgets
            ActiveTextWidgets.RemoveAt(i);
            // Remove the time remaining from the active time remaining
            ActiveTimeRemaining.RemoveAt(i);
            // Remove the opacity from the active opacity
            ActiveOpacity.RemoveAt(i);
        }
    }
}


