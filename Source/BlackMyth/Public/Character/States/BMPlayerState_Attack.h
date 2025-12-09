#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BMPlayerState_Attack.generated.h"

class UAnimInstance;
class UAnimMontage;

UCLASS()
class BLACKMYTH_API UBMPlayerState_Attack : public UBMCharacterState
{
    GENERATED_BODY()

public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnExit(float DeltaTime) override;
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    void FinishAttack(bool bInterrupted);
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
    TWeakObjectPtr<UAnimInstance> CachedAnim;
    TObjectPtr<UAnimMontage> PlayingMontage = nullptr;
    FTimerHandle TimerHandle;
    bool bFinished = false;

    // 保存进入攻击前的参数
    float SavedGroundFriction = 0.f;
    float SavedBrakingDecelWalking = 0.f;
    float SavedBrakingFrictionFactor = 0.f;
    bool  bSavedUseSeparateBrakingFriction = false;
    float SavedBrakingFriction = 0.f;

    void ApplyAttackInertiaSettings(UCharacterMovementComponent* Move);
    void RestoreMovementSettings(UCharacterMovementComponent* Move);
};
