#include "Character/Enemy/States/BMEnemyState_Hit.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Core/BMTypes.h"

/*
 * @brief On enter, it enters the hit state, it stops the enemy movement, plays the hit animation and sets the hit finish handle
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Hit::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    bFinished = false;
    E->GetWorldTimerManager().ClearTimer(HitFinishHandle);

    // �ܻ�ʱֹͣ·������
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

/*
 * @brief On exit, it exits the hit state, it clears the hit finish handle
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Hit::OnExit(float)
{
    if (ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext()))
    {
        E->GetWorldTimerManager().ClearTimer(HitFinishHandle);
    }
    bFinished = false;
}

/*
 * @brief Can transition to, it checks if the state can transition to the given state
 * @param StateName The name of the state to transition to
 * @return True if the state can transition to the given state, false otherwise
 */
bool UBMEnemyState_Hit::CanTransitionTo(FName StateName) const
{
    // �ܻ����Ա�����ǿ�ƴ��
    if (StateName == BMEnemyStateNames::Death) return true;
    return bFinished;
}

/*
 * @brief Finish hit, it finishes the hit, it changes the state to chase or patrol or idle
 */
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
