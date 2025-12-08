#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
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
};
