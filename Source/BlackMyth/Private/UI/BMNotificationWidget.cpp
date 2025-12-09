// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMNotificationWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "System/Event/BMEventBusSubsystem.h"

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
    UTextBlock* Text = NewObject<UTextBlock>(this);
    if (Text)
    {
        Text->SetText(Msg);
        MessageList->AddChildToVerticalBox(Text);
    }
}

