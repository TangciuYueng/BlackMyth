#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BMEnemyAIController.generated.h"


class UBlackboardComponent;
class UBehaviorTreeComponent;

UCLASS()
class BLACKMYTH_API ABMEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABMEnemyAIController();

    virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* InPawn) override;

    bool RequestMoveToActor(AActor* Target, float AcceptanceRadius);
    bool RequestMoveToLocation(const FVector& Location, float AcceptanceRadius);
    void RequestStopMovement();
    bool IsMoveActive() const;

public:
    UPROPERTY(VisibleAnywhere, Category = "BM|AI")
    TObjectPtr<UBlackboardComponent> BlackboardComp;

    UPROPERTY(VisibleAnywhere, Category = "BM|AI")
    TObjectPtr<UBehaviorTreeComponent> BehaviorComp;
};