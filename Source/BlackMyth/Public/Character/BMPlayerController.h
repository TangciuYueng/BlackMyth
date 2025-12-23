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
    void ConsumeStaminaTest(); // [TEST] Consume 25 stamina for testing

    // Skill cooldown testing
    void TriggerSkillCooldown(FName SkillId, float TotalSeconds);
    void OnSkillCooldownTick_Skill1();
    void OnSkillCooldownTick_Skill2();
    void StartSkill1Cooldown();
    void StartSkill2Cooldown();

    FTimerHandle Skill1CooldownTimer;
    FTimerHandle Skill2CooldownTimer;
    float Skill1Remaining = 0.f;
    float Skill2Remaining = 0.f;
};
