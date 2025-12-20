#pragma once

#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "BMEnemyState_Death.generated.h"

UCLASS()
class BLACKMYTH_API UBMEnemyState_Death : public UBMCharacterState
{
    GENERATED_BODY()
public:
    virtual void OnEnter(float DeltaTime) override;
    virtual bool CanTransitionTo(FName StateName) const override { return false; }

private:
    void FinishDeath();

private:
    FTimerHandle DeathFinishHandle;
};
