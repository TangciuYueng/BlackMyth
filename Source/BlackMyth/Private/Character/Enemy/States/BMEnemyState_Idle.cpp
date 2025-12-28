#include "Character/Enemy/States/BMEnemyState_Idle.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Core/BMTypes.h"
#include "GameFramework/CharacterMovementComponent.h"

/*
 * @brief On enter, it enters the idle state, it stops the enemy movement, plays the idle animation and sets the max walk speed to the patrol speed
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Idle::OnEnter(float)
{
    if (ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext()))
    {
        E->RequestStopMovement();
        E->PlayIdleLoop();
        if (auto* Move = E->GetCharacterMovement()) Move->MaxWalkSpeed = E->GetPatrolSpeed();
    }

}

/*
 * @brief On update, it updates the idle state, it checks if the enemy is alerted and has a valid target
 * if not, it changes the state to patrol or chase
 * @param DeltaTime The delta time
 */
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
