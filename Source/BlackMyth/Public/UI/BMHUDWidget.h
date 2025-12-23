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
    UPROPERTY(meta=(BindWidget)) class UProgressBar* StaminaBar = nullptr;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Skill1CooldownText = nullptr;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Skill2CooldownText = nullptr;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Skill3CooldownText = nullptr;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* LevelText = nullptr; // 显示当前等级

protected:
    virtual void BindEventBus(class UBMEventBusSubsystem* EventBus) override;
    virtual void UnbindEventBus(class UBMEventBusSubsystem* EventBus) override;

private:
    // Cached bindings
    FDelegateHandle HealthChangedHandle;
    FDelegateHandle ManaChangedHandle;
    FDelegateHandle SkillCooldownHandle;
    FDelegateHandle LevelChangedHandle;

    void HandleHealthChanged(float Normalized);
    void HandleStaminaChanged(float Normalized);
    void HandleSkillCooldownChanged(FName SkillId, float RemainingSeconds);
    void HandleLevelChanged(int32 NewLevel);

    // Format cooldown according to UX rules. Returns empty when ready.
    FText FormatCooldownText(float RemainingSeconds) const;
};
