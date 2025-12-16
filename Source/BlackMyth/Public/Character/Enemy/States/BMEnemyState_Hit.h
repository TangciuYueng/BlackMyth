#pragma once

#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "BMEnemyState_Hit.generated.h"

UCLASS()
class BLACKMYTH_API UBMEnemyState_Hit : public UBMCharacterState
{
    GENERATED_BODY()
public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnExit(float DeltaTime) override;
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    void FinishHit();

private:
    FTimerHandle HitFinishHandle;
    bool bFinished = false;
};
