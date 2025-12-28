    // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UProgressBar;
class UTextBlock;
class UBMExperienceComponent;
#include "BMHUDWidget.generated.h"

/**
 * @brief Define the FSkillCooldownData struct
 * @param FSkillCooldownData The name of the struct
 * @param UBMWidgetBase The parent class
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
 * @brief Define the UBMHUDWidget class
 * @param UBMHUDWidget The name of the class
 * @param UBMWidgetBase The parent class
 */
UCLASS()
class BLACKMYTH_API UBMHUDWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    // Bound from WBP_HUD
    UPROPERTY(meta=(BindWidget)) class UProgressBar* HealthBar = nullptr;
    // Stamina bar binding
    UPROPERTY(meta=(BindWidget)) class UProgressBar* StaminaBar = nullptr;
    // Skill 1 cooldown text binding
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Skill1CooldownText = nullptr;
    // Skill 2 cooldown text binding
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Skill2CooldownText = nullptr;
    // Skill 3 cooldown text binding
    UPROPERTY(meta=(BindWidget)) class UTextBlock* Skill3CooldownText = nullptr;
    // Level text binding
    UPROPERTY(meta=(BindWidget)) class UTextBlock* LevelText = nullptr;

protected:
    // Bind event bus
    virtual void BindEventBus(class UBMEventBusSubsystem* EventBus) override;
    // Unbind event bus
    virtual void UnbindEventBus(class UBMEventBusSubsystem* EventBus) override;
    // Native tick
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

    // Handle health changed
    void HandleHealthChanged(float Normalized);
    // Handle mana changed
    void HandleManaChanged(float Normalized);
    // Handle stamina changed
    void HandleStaminaChanged(float Normalized);
    // Handle skill cooldown changed
    void HandleSkillCooldownChanged(FName SkillId, float RemainingSeconds);
    // Handle level changed
    void HandleLevelChanged(int32 NewLevel);

    // Sync initial values
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
