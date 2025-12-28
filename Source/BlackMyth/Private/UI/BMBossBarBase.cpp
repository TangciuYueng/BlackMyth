// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMBossBarBase.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "System/Event/BMEventBusSubsystem.h"

/*
 * @brief Native construct, it native constructs the boss bar base
 */
void UBMBossBarBase::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Hide boss bar by default - it will be shown when boss becomes alert
    SetVisibility(ESlateVisibility::Collapsed);
    
    // Configure health bar colors: red fill for health, gray for empty portion
    if (BossHealthBar)
    {
        BossHealthBar->SetFillColorAndOpacity(FLinearColor::Red);
        BossHealthBar->WidgetStyle.BackgroundImage.TintColor = FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));
        BossHealthBar->SetPercent(1.0f);
    }
}

/*
 * @brief Bind event bus, it binds the event bus
 * @param EventBus The event bus
 */
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

/*
 * @brief Unbind event bus, it unbinds the event bus
 * @param EventBus The event bus
 */
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

/*
 * @brief Handle boss health, it handles the boss health
 * @param Normalized The normalized
 */
void UBMBossBarBase::HandleBossHealth(float Normalized)
{
    if (BossHealthBar)
    {
        BossHealthBar->SetPercent(FMath::Clamp(Normalized, 0.f, 1.f));
    }
}

/*
 * @brief Handle boss phase, it handles the boss phase
 * @param Phase The phase
 * @param Hint The hint
 */
void UBMBossBarBase::HandleBossPhase(int32 Phase, const FText& Hint)
{
    // Show boss bar when phase changes (boss becomes alert)
    if (!IsVisible())
    {
        SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    
    if (PhaseHintText)
    {
        const FText PhaseText = FText::Format(NSLOCTEXT("BM", "BossPhaseFmt", "Phase {0}: {1}"), FText::AsNumber(Phase), Hint);
        PhaseHintText->SetText(PhaseText);
    }
}

