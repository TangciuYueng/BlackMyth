    // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UProgressBar;
class UTextBlock;
class UBMExperienceComponent;
#include "BMHUDWidget.generated.h"

/**
 * 技能冷却数据结构
 */
USTRUCT()
struct FSkillCooldownData
{
    GENERATED_BODY()

    /** 冷却结束的时间戳（World Time） */
    float CooldownEndTime = 0.f;

    /** 冷却总时长 */
    float TotalCooldown = 0.f;

    /** 是否正在冷却中 */
    bool bIsCoolingDown = false;
};

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
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    // Cached bindings
    FDelegateHandle HealthChangedHandle;
    FDelegateHandle ManaChangedHandle;
    FDelegateHandle StaminaChangedHandle;
    FDelegateHandle SkillCooldownHandle;
    FDelegateHandle LevelChangedHandle;
    // Direct binding to XP component (native delegate) as a fallback
    TWeakObjectPtr<UBMExperienceComponent> CachedXP;
    FDelegateHandle XPLevelUpHandle;

    void HandleHealthChanged(float Normalized);
    void HandleManaChanged(float Normalized);
    void HandleStaminaChanged(float Normalized);
    void HandleSkillCooldownChanged(FName SkillId, float RemainingSeconds);
    void HandleLevelChanged(int32 NewLevel);

    void SyncInitialValues();

    // Format cooldown according to UX rules. Returns empty when ready.
    FText FormatCooldownText(float RemainingSeconds) const;

    /** 更新所有技能的冷却显示 */
    void UpdateCooldownDisplays(float DeltaTime);
    
    /** 更新单个技能的冷却显示 */
    void UpdateSingleCooldownDisplay(FName SkillId, FSkillCooldownData& CooldownData, UTextBlock* TextWidget);
    
    /** 获取当前世界时间 */
    float GetCurrentWorldTime() const;

    /** 技能冷却数据映射表 */
    UPROPERTY(Transient)
    TMap<FName, FSkillCooldownData> SkillCooldowns;
};
