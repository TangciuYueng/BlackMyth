#include "Character/Enemy/BMEnemyAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

ABMEnemyAIController::ABMEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
}

void ABMEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
}

void ABMEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // 可选：同步黑板（不做行为决策），你后续接 BT 也方便
}

bool ABMEnemyAIController::RequestMoveToActor(AActor* Target, float AcceptanceRadius)
{
    if (!Target) return false;
    MoveToActor(Target, AcceptanceRadius, true, true, true, nullptr, true);
    return true;
}

bool ABMEnemyAIController::RequestMoveToLocation(const FVector& Location, float AcceptanceRadius)
{
    MoveToLocation(Location, AcceptanceRadius, true, true, false, true);
    return true;
}

void ABMEnemyAIController::RequestStopMovement()
{
    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
}

bool ABMEnemyAIController::IsMoveActive() const
{
    const UPathFollowingComponent* P = GetPathFollowingComponent();
    if (!P) return false;
    return P->GetStatus() == EPathFollowingStatus::Moving;
}