// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "System/Event/BMEventBusSubsystem.h"

void UBMHUDWidget::BindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;

    if (!HealthChangedHandle.IsValid())
    {
        HealthChangedHandle = EventBus->OnPlayerHealthChanged.AddWeakLambda(this, [this](float Normalized)
        {
            HandleHealthChanged(Normalized);
        });
    }
    if (!ManaChangedHandle.IsValid())
    {
        ManaChangedHandle = EventBus->OnPlayerManaChanged.AddWeakLambda(this, [this](float Normalized)
        {
            HandleStaminaChanged(Normalized);
        });
    }
    if (!SkillCooldownHandle.IsValid())
    {
        SkillCooldownHandle = EventBus->OnSkillCooldownChanged.AddWeakLambda(this, [this](FName SkillId, float RemainingSeconds)
        {
            HandleSkillCooldownChanged(SkillId, RemainingSeconds);
        });
    }
}

void UBMHUDWidget::UnbindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;

    if (HealthChangedHandle.IsValid())
    {
        EventBus->OnPlayerHealthChanged.Remove(HealthChangedHandle);
        HealthChangedHandle.Reset();
    }
    if (ManaChangedHandle.IsValid())
    {
        EventBus->OnPlayerManaChanged.Remove(ManaChangedHandle);
        ManaChangedHandle.Reset();
    }
    if (SkillCooldownHandle.IsValid())
    {
        EventBus->OnSkillCooldownChanged.Remove(SkillCooldownHandle);
        SkillCooldownHandle.Reset();
    }
}

void UBMHUDWidget::HandleHealthChanged(float Normalized)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(FMath::Clamp(Normalized, 0.f, 1.f));
    }
}

void UBMHUDWidget::HandleStaminaChanged(float Normalized)
{
    if (StaminaBar)
    {
        StaminaBar->SetPercent(FMath::Clamp(Normalized, 0.f, 1.f));
    }
}

void UBMHUDWidget::HandleSkillCooldownChanged(FName SkillId, float RemainingSeconds)
{
    const FText DisplayText = FormatCooldownText(RemainingSeconds);
    if (SkillId == TEXT("Skill1"))
    {
        if (Skill1CooldownText)
        {
            Skill1CooldownText->SetText(DisplayText);
            Skill1CooldownText->SetVisibility(DisplayText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
        }
    }
    else if (SkillId == TEXT("Skill2"))
    {
        if (Skill2CooldownText)
        {
            Skill2CooldownText->SetText(DisplayText);
            Skill2CooldownText->SetVisibility(DisplayText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
        }
    }
}

FText UBMHUDWidget::FormatCooldownText(float RemainingSeconds) const
{
    const float Clamped = FMath::Max(0.f, RemainingSeconds);
    if (Clamped <= 0.05f)
    {
        return FText::GetEmpty();
    }

    if (Clamped < 10.f)
    {
        // One decimal below 10s
        return FText::AsNumber(FMath::RoundToFloat(Clamped * 10.f) / 10.f);
    }

    if (Clamped < 60.f)
    {
        // Integer seconds
        return FText::AsNumber(FMath::RoundToInt(Clamped));
    }

    const int32 Total = FMath::RoundToInt(Clamped);
    const int32 Minutes = Total / 60;
    const int32 Seconds = Total % 60;
    FNumberFormattingOptions Opts;
    Opts.MinimumIntegralDigits = 2;
    const FText SecText = FText::AsNumber(Seconds, &Opts);
    return FText::FromString(FString::Printf(TEXT("%d:%s"), Minutes, *SecText.ToString()));
}

