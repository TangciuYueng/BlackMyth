// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UProgressBar;
class UTextBlock;
#include "BMHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMHUDWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    // Bound from WBP_HUD
    UPROPERTY(meta=(BindWidget)) class UProgressBar* HealthBar = nullptr;
    UPROPERTY(meta=(BindWidget)) class UProgressBar* ManaBar = nullptr;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Skill1CooldownText = nullptr;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Skill2CooldownText = nullptr;

protected:
    virtual void BindEventBus(class UBMEventBusSubsystem* EventBus) override;
    virtual void UnbindEventBus(class UBMEventBusSubsystem* EventBus) override;

private:
    // Cached bindings
    FDelegateHandle HealthChangedHandle;
    FDelegateHandle ManaChangedHandle;
    FDelegateHandle SkillCooldownHandle;

    void HandleHealthChanged(float Normalized);
    void HandleManaChanged(float Normalized);
    void HandleSkillCooldownChanged(FName SkillId, float RemainingSeconds);
};
