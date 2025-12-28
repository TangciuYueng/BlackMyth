// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BMPlayerController.generated.h"

class UBMBookWidget;

/**
 * @brief Define the BMPlayerController class
 * @param ABMPlayerController The name of the class
 * @param APlayerController The parent class
 */
UCLASS()
class BLACKMYTH_API ABMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    ABMPlayerController(); // Add constructor

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Call this when the intro video finishes
    UFUNCTION(BlueprintCallable, Category = "BM|UI")
    void OnIntroVideoFinished();

private:
    // Show the main menu
    void ShowMainMenu();
    // Toggle the pause menu
    void TogglePauseMenu();
    // Apply half HP damage
    void ApplyHalfHPDamage();
    // Show a test notification message via UBMNotifications subsystem
    void DebugShowNotification();
    // Consume stamina test
    void ConsumeStaminaTest();

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
    void Input_EnterPressed();

    // Skill cooldown timers
    FTimerHandle Skill1CooldownTimer;
    FTimerHandle Skill2CooldownTimer;
    // Skill 1 remaining time
    float Skill1Remaining = 0.f;
    // Skill 2 remaining time
    float Skill2Remaining = 0.f;
    FTimerHandle Skill3CooldownTimer;
    float Skill3Remaining = 0.f;

    // Editable test message for notification
    UPROPERTY(EditAnywhere, Category="BM|Debug")
    FString MessageTest = TEXT("Test Notification");
    // Book UI
    UPROPERTY(EditDefaultsOnly, Category = "BM|UI")
    TSubclassOf<UBMBookWidget> BookWidgetClass;
    // Book widget instance
    UPROPERTY()
    UBMBookWidget* BookWidgetInstance = nullptr;

    bool bHasShownIntroBook = false;

    void ShowBookUI();
};
