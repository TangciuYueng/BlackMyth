#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "BMEnemyBossState_PhaseChange.generated.h"

UCLASS()
class BLACKMYTH_API UBMEnemyBossState_PhaseChange : public UBMCharacterState
{
    GENERATED_BODY()

public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnExit(float DeltaTime) override;
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    void StartHoldAfterDeath();
    void StartDeathReverse();
    void StartEnergize();
    void FinishPhaseChange();

private:
    FTimerHandle StepTimer;
    bool bFinished = false;
};
