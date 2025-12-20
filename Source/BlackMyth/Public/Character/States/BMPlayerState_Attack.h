#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "BMPlayerState_Attack.generated.h"

UCLASS()
class BLACKMYTH_API UBMPlayerState_Attack : public UBMCharacterState
{
    GENERATED_BODY()

public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnExit(float DeltaTime) override;
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    void StartComboStep(int32 StepIndex);
    void StartSkill(const struct FBMPlayerAttackSpec& Spec);

    void OpenLinkWindow();
    void CloseLinkWindow();
    void PollComboInput();

    void OnStepFinished();
    void StartRecoverForStep(int32 FromStepIndex);
    void OnRecoverFinished();

    void FinishAttack(bool bInterrupted);

    void ApplyAttackInertiaSettings(class UCharacterMovementComponent* Move);
    void RestoreMovementSettings(class UCharacterMovementComponent* Move);

private:
    FTimerHandle TimerStepEnd;
    FTimerHandle TimerWindowOpen;
    FTimerHandle TimerWindowClose;
    FTimerHandle TimerPollInput;
    FTimerHandle TimerRecoverEnd;

    bool bFinished = false;

    // combo runtime
    bool bIsCombo = false;
    int32 ComboIndex = -1;

    bool bLinkWindowOpen = false;
    bool bQueuedNext = false;

    // inertia cache
    float SavedGroundFriction = 0.f;
    float SavedBrakingDecelWalking = 0.f;
    float SavedBrakingFrictionFactor = 0.f;
    bool  bSavedUseSeparateBrakingFriction = false;
    float SavedBrakingFriction = 0.f;
};
