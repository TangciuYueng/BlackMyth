#include "Character/Enemy/States/BMEnemyState_Chase.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Enemy/BMEnemyAIController.h"
#include "Core/BMTypes.h"
#include "GameFramework/CharacterMovementComponent.h"

/*
 * @brief On enter, it enters the chase state, it plays the run loop and sets the max walk speed to the chase speed
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Chase::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    E->PlayRunLoop();
    if (auto* Move = E->GetCharacterMovement()) Move->MaxWalkSpeed = E->GetChaseSpeed();

}

/*
 * @brief On update, it updates the chase state, it checks if the enemy is alerted and has a valid target
 * if not, it changes the state to patrol or idle, if the enemy is in attack range, it changes the state to attack, if the enemy is not in attack range, it moves to the target
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Chase::OnUpdate(float DeltaTime)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    if (!E->IsAlerted() || !E->HasValidTarget())
    {
        if (E->GetFSM()) E->GetFSM()->ChangeStateByName(E->GetPatrolRadius() > 0.f ? BMEnemyStateNames::Patrol : BMEnemyStateNames::Idle);
        return;
    }

    // ���������룺����ܳ��� -> Attack������ͣ��������Ŀ�겢��Idle
    if (E->IsInAttackRange())
    {
        E->RequestStopMovement();
        E->FaceTarget(DeltaTime);

        if (E->CanStartAttack())
            E->GetFSM()->ChangeStateByName(BMEnemyStateNames::Attack);
        else
            E->PlayIdleLoop();

        return;
    }

    // ׷��
    E->RequestMoveToTarget(1.f);

    // ����Ŀ��
    E->FaceTarget(DeltaTime);


    const float Speed2D = E->GetVelocity().Size2D();
    if (Speed2D <= E->GetLocomotionSpeedThreshold())
        E->PlayIdleLoop();
    else
        E->PlayRunLoop();
}