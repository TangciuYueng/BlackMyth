// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BMPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API ABMPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    void ShowMainMenu();
    void TogglePauseMenu();
    void ApplyHalfHPDamage();

    // Skill cooldown testing
    void TriggerSkillCooldown(FName SkillId, float TotalSeconds);
    void OnSkillCooldownTick_Skill1();
    void OnSkillCooldownTick_Skill2();
    void StartSkill1Cooldown();
    void StartSkill2Cooldown();
    void StartSkill3Cooldown();

    // Test: add enough XP to gain exactly one level for HUD verification
    void DebugGainOneLevel();
    void OnSkillCooldownTick_Skill3();

    FTimerHandle Skill1CooldownTimer;
    FTimerHandle Skill2CooldownTimer;
    float Skill1Remaining = 0.f;
    float Skill2Remaining = 0.f;
    FTimerHandle Skill3CooldownTimer;
    float Skill3Remaining = 0.f;
};
