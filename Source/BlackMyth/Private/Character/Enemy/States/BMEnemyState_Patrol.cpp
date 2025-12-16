#include "Character/Enemy/States/BMEnemyState_Patrol.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Enemy/BMEnemyAIController.h"
#include "Core/BMTypes.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBMEnemyState_Patrol::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    E->PlayWalkLoop();
    if (auto* Move = E->GetCharacterMovement()) Move->MaxWalkSpeed = E->GetPatrolSpeed();

    RepathAccum = 999.f; // 立刻选点
}

void UBMEnemyState_Patrol::OnUpdate(float DeltaTime)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    // 发现玩家 -> Chase（只保留一次）
    if (E->IsAlerted() && E->HasValidTarget())
    {
        E->GetFSM()->ChangeStateByName(BMEnemyStateNames::Chase);
        return;
    }

    // 1) 每帧做动画门控（不要放在 RepathAccum 的 return 后面）
    const float Speed2D = E->GetVelocity().Size2D();
    if (Speed2D <= E->GetLocomotionSpeedThreshold())
        E->PlayIdleLoop();
    else
        E->PlayWalkLoop();

    // 2) 仅每隔一段时间重新选点并发 MoveTo
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