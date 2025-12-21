#include "Character/Enemy/States/BMEnemyState_Chase.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Enemy/BMEnemyAIController.h"
#include "Core/BMTypes.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBMEnemyState_Chase::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    E->PlayRunLoop();
    if (auto* Move = E->GetCharacterMovement()) Move->MaxWalkSpeed = E->GetChaseSpeed();

}

void UBMEnemyState_Chase::OnUpdate(float DeltaTime)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    if (!E->IsAlerted() || !E->HasValidTarget())
    {
        if (E->GetFSM()) E->GetFSM()->ChangeStateByName(E->GetPatrolRadius() > 0.f ? BMEnemyStateNames::Patrol : BMEnemyStateNames::Idle);
        return;
    }

    // 进攻击距离：如果能出手 -> Attack；否则停下来面向目标并播Idle（避免原地跑）
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

    // 追击
    E->RequestMoveToTarget(1.f);

    // 面向目标
    E->FaceTarget(DeltaTime);


    const float Speed2D = E->GetVelocity().Size2D();
    if (Speed2D <= E->GetLocomotionSpeedThreshold())
        E->PlayIdleLoop();
    else
        E->PlayRunLoop();
}