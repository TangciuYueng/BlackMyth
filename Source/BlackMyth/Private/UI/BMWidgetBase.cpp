// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMWidgetBase.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"

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

