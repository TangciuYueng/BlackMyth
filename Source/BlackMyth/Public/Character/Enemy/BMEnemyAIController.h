// ABMEnemyAIController.h
#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "ABMEnemyAIController.generated.h"

// 前置声明
class UBehaviorTreeComponent;
class UBlackboardComponent;

UCLASS()
class MYGAME_API AABMEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AABMEnemyAIController();

    // OnPossess(APawn* InPawn) void
    virtual void OnPossess(APawn* InPawn) override;

    // Tick(float) void
    virtual void Tick(float DeltaTime) override;

    // 获取 Blackboard 组件的访问器
    FORCEINLINE UBlackboardComponent* GetBlackboard() const { return Blackboard; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComp;
};