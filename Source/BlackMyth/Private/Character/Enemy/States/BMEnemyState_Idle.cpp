#include "Character/Enemy/States/BMEnemyState_Idle.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Core/BMTypes.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBMEnemyState_Idle::OnEnter(float)
{
    if (ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext()))
    {
        E->RequestStopMovement();
        E->PlayIdleLoop();
        if (auto* Move = E->GetCharacterMovement()) Move->MaxWalkSpeed = E->GetPatrolSpeed();
    }

}

void UBMEnemyState_Idle::OnUpdate(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    if (E->IsAlerted() && E->HasValidTarget())
    {
        if (E->GetFSM()) E->GetFSM()->ChangeStateByName(BMEnemyStateNames::Chase);
        return;
    }

    if (!E->IsAlerted() && E->GetPatrolRadius() > 0.f)
    {
        if (E->GetFSM()) E->GetFSM()->ChangeStateByName(BMEnemyStateNames::Patrol);
        return;
    }
}
