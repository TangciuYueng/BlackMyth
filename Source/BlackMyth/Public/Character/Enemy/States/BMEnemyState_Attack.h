#pragma once
#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "Core/BMTypes.h"
#include "BMEnemyState_Attack.generated.h"


UCLASS()
class BLACKMYTH_API UBMEnemyState_Attack : public UBMCharacterState
{
    GENERATED_BODY()

public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnExit(float DeltaTime) override;
    virtual void OnUpdate(float DeltaTime) override;
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    void FinishAttack();

private:
    FTimerHandle AttackFinishHandle;
    bool bFinished = false;

    // 本次选择的攻击
    FBMEnemyAttackSpec ActiveAttack;
};