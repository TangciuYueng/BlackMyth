// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMBossBarBase.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "System/Event/BMEventBusSubsystem.h"

void UBMBossBarBase::BindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;

    if (!BossHealthHandle.IsValid())
    {
        BossHealthHandle = EventBus->OnBossHealthChanged.AddWeakLambda(this, [this](float Normalized)
        {
            HandleBossHealth(Normalized);
        });
    }
    if (!BossPhaseHandle.IsValid())
    {
        BossPhaseHandle = EventBus->OnBossPhaseChanged.AddWeakLambda(this, [this](int32 Phase, const FText& Hint)
        {
            HandleBossPhase(Phase, Hint);
        });
    }
}

void UBMBossBarBase::UnbindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;

    if (BossHealthHandle.IsValid())
    {
        EventBus->OnBossHealthChanged.Remove(BossHealthHandle);
        BossHealthHandle.Reset();
    }
    if (BossPhaseHandle.IsValid())
    {
        EventBus->OnBossPhaseChanged.Remove(BossPhaseHandle);
        BossPhaseHandle.Reset();
    }
}

void UBMBossBarBase::HandleBossHealth(float Normalized)
{
    if (BossHealthBar)
    {
        BossHealthBar->SetPercent(FMath::Clamp(Normalized, 0.f, 1.f));
    }
}

void UBMBossBarBase::HandleBossPhase(int32 Phase, const FText& Hint)
{
    if (PhaseHintText)
    {
        const FText PhaseText = FText::Format(NSLOCTEXT("BM", "BossPhaseFmt", "Phase {0}: {1}"), FText::AsNumber(Phase), Hint);
        PhaseHintText->SetText(PhaseText);
    }
}

