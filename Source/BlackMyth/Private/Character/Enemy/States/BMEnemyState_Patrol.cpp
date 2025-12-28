#include "Character/Enemy/States/BMEnemyState_Patrol.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Enemy/BMEnemyAIController.h"
#include "Core/BMTypes.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"

/*
 * @brief On enter, it enters the patrol state, it plays the walk loop and sets the max walk speed to the patrol speed
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Patrol::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    E->PlayWalkLoop();
    if (auto* Move = E->GetCharacterMovement()) Move->MaxWalkSpeed = E->GetPatrolSpeed();

    RepathAccum = 999.f; // ����ѡ��
}

/*
 * @brief On update, it updates the patrol state, it checks if the enemy is alerted and has a valid target
 * if not, it changes the state to chase
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Patrol::OnUpdate(float DeltaTime)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    // ������� -> Chase
    if (E->IsAlerted() && E->HasValidTarget())
    {
        E->GetFSM()->ChangeStateByName(BMEnemyStateNames::Chase);
        return;
    }

    // ÿ֡�������ſ�
    const float Speed2D = E->GetVelocity().Size2D();
    if (Speed2D <= E->GetLocomotionSpeedThreshold())
        E->PlayIdleLoop();
    else
        E->PlayWalkLoop();

    // ��ÿ��һ��ʱ������ѡ�㲢�� MoveTo
    RepathAccum += DeltaTime;
    if (RepathAccum < 2.0f) return;
    RepathAccum = 0.f;

    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(E->GetWorld());
    if (!Nav) return;

    FNavLocation Out;
    if (Nav->GetRandomPointInNavigableRadius(E->GetHomeLocation(), E->GetPatrolRadius(), Out))
    {
        PatrolDest = Out.Location;
        E->RequestMoveToLocation(PatrolDest, 80.f);
    }
}