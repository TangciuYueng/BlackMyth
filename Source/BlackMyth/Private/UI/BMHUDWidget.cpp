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
            HandleManaChanged(Normalized);
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

void UBMHUDWidget::HandleManaChanged(float Normalized)
{
    if (ManaBar)
    {
        ManaBar->SetPercent(FMath::Clamp(Normalized, 0.f, 1.f));
    }
}

void UBMHUDWidget::HandleSkillCooldownChanged(FName SkillId, float RemainingSeconds)
{
    const FString Display = FString::Printf(TEXT("%.1fs"), FMath::Max(0.f, RemainingSeconds));
    if (SkillId == TEXT("Skill1"))
    {
        if (Skill1CooldownText)
        {
            Skill1CooldownText->SetText(FText::FromString(Display));
        }
    }
    else if (SkillId == TEXT("Skill2"))
    {
        if (Skill2CooldownText)
        {
            Skill2CooldownText->SetText(FText::FromString(Display));
        }
    }
}

