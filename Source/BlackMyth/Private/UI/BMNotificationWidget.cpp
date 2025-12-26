// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMNotificationWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "Blueprint/WidgetTree.h"


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

void UBMNotificationWidget::UnbindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;
    if (NotifyHandle.IsValid())
    {
        EventBus->OnNotifyMessage.Remove(NotifyHandle);
        NotifyHandle.Reset();
    }
}

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

void UBMNotificationWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (ActiveTextWidgets.Num() == 0) return;

    for (int32 i = ActiveTextWidgets.Num() - 1; i >= 0; --i)
    {
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

        Op = FMath::Max(0.f, Op - FadeSpeed * InDeltaTime);
        if (UTextBlock* TB = TextPtr.Get())
        {
            FSlateColor Color = TB->ColorAndOpacity;
            FLinearColor Linear = Color.GetSpecifiedColor();
            Linear.A = Op;
            TB->SetColorAndOpacity(FSlateColor(Linear));
        }

        if (Op <= KINDA_SMALL_NUMBER)
        {
            if (UWidget* W = TextPtr.Get())
            {
                MessageList->RemoveChild(W);
            }
            ActiveTextWidgets.RemoveAt(i);
            ActiveTimeRemaining.RemoveAt(i);
            ActiveOpacity.RemoveAt(i);
        }
    }
}


