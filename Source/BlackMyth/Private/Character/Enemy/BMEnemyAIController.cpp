#include "Character/Enemy/BMEnemyAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

/*
 * @brief Constructor of the ABMEnemyAIController class
 */
ABMEnemyAIController::ABMEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
}

/*
 * @brief On possess, it possesses the enemy AI controller
 * @param InPawn The pawn to possess
 */
void ABMEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
}

/*
 * @brief Tick, it ticks the enemy AI controller
 * @param DeltaSeconds The delta seconds
 */
void ABMEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

/*
 * @brief Request move to actor, it requests the enemy AI controller to move to the target actor
 * @param Target The target actor
 * @param AcceptanceRadius The acceptance radius
 * @return True if the request is successful, false otherwise
 */
bool ABMEnemyAIController::RequestMoveToActor(AActor* Target, float AcceptanceRadius)
{
    if (!Target) return false;
    MoveToActor(Target, AcceptanceRadius, true, true, true, nullptr, true);
    return true;
}

/*
 * @brief Request move to location, it requests the enemy AI controller to move to the target location
 * @param Location The target location
 * @param AcceptanceRadius The acceptance radius
 * @return True if the request is successful, false otherwise
 */
bool ABMEnemyAIController::RequestMoveToLocation(const FVector& Location, float AcceptanceRadius)
{
    MoveToLocation(Location, AcceptanceRadius, true, true, false, true);
    return true;
}

/*
 * @brief Request stop movement, it requests the enemy AI controller to stop movement
 */
void ABMEnemyAIController::RequestStopMovement()
{
    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
}

/*
 * @brief Is move active, it checks if the enemy AI controller is moving
 * @return True if the enemy AI controller is moving, false otherwise
 */
bool ABMEnemyAIController::IsMoveActive() const
{
    const UPathFollowingComponent* P = GetPathFollowingComponent();
    if (!P) return false;
    return P->GetStatus() == EPathFollowingStatus::Moving;
}