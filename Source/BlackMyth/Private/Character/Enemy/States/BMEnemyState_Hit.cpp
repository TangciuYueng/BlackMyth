#include "Character/Enemy/States/BMEnemyState_Hit.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Core/BMTypes.h"

void UBMEnemyState_Hit::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    bFinished = false;
    E->GetWorldTimerManager().ClearTimer(HitFinishHandle);

    // 受击时停止路径跟随
    E->RequestStopMovement();

    const float Duration = E->PlayHitOnce(E->GetLastDamageInfo());
    if (Duration <= 0.f)
    {
        FinishHit();
        return;
    }

    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]() { FinishHit(); });
    E->GetWorldTimerManager().SetTimer(HitFinishHandle, D, Duration, false);
}

void UBMEnemyState_Hit::OnExit(float)
{
    if (ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext()))
    {
        E->GetWorldTimerManager().ClearTimer(HitFinishHandle);
    }
    bFinished = false;
}

bool UBMEnemyState_Hit::CanTransitionTo(FName StateName) const
{
    // 受击可以被死亡强制打断
    if (StateName == BMEnemyStateNames::Death) return true;
    return bFinished;
}

void UBMEnemyState_Hit::FinishHit()
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    bFinished = true;

    if (UBMStateMachineComponent* FSM = E->GetFSM())
    {
        if (E->IsAlerted() && E->HasValidTarget())
            FSM->ChangeStateByName(BMEnemyStateNames::Chase);
        else if (E->GetPatrolRadius() > 0.f)
            FSM->ChangeStateByName(BMEnemyStateNames::Patrol);
        else
            FSM->ChangeStateByName(BMEnemyStateNames::Idle);
    }
}
