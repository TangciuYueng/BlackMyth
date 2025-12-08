#include "ABMEnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "ABMEnemyBase.h"
#include "BMTypes.h" 

AABMEnemyAIController::AABMEnemyAIController()
{
    // 创建组件
    BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void AABMEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AABMEnemyBase* Enemy = Cast<AABMEnemyBase>(InPawn);
    if (Enemy && Enemy->BehaviorTreeAsset)
    {
        // 1. 初始化黑板数据
        if (Enemy->BehaviorTreeAsset->BlackboardAsset)
        {
            BlackboardComp->InitializeBlackboard(*Enemy->BehaviorTreeAsset->BlackboardAsset);
        }

        // 2. 设置自身的引用 (SelfActor)
        BlackboardComp->SetValueAsObject(BMBlackboardKeys::TargetActor, nullptr); // 初始清空
        
        // 3. 记录出生点 (HomeLocation)
        BlackboardComp->SetValueAsVector(BMBlackboardKeys::HomeLocation, InPawn->GetActorLocation());

        // 4. 启动行为树
        BehaviorTreeComp->StartTree(*Enemy->BehaviorTreeAsset);
    }
}

void AABMEnemyAIController::OnUnPossess()
{
    Super::OnUnPossess();
    
    // 停止行为树逻辑
    if (BehaviorTreeComp)
    {
        BehaviorTreeComp->StopTree();
    }
}

void AABMEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 在纯 C++ 架构中，我们可以在 Controller Tick 里做感知更新
    // 也可以使用 BTService。这里演示在 Controller 统一更新，便于管理。
    UpdatePerception();
}

void AABMEnemyAIController::UpdatePerception()
{
    AABMEnemyBase* Enemy = Cast<AABMEnemyBase>(GetPawn());
    if (!Enemy || !BlackboardComp) return;

    // 1. 调用 EnemyBase 的物理检测能力
    AActor* DetectedTarget = Enemy->DetectPlayer();

    if (DetectedTarget)
    {
        // === 发现玩家 ===
        
        // 更新目标对象
        BlackboardComp->SetValueAsObject(BMBlackboardKeys::TargetActor, DetectedTarget);
        
        // 更新目标位置
        BlackboardComp->SetValueAsVector(BMBlackboardKeys::TargetLocation, DetectedTarget->GetActorLocation());

        // 计算距离并写入 Blackboard (供 Decorator 判断是否攻击)
        float Dist = FVector::Dist(Enemy->GetActorLocation(), DetectedTarget->GetActorLocation());
        BlackboardComp->SetValueAsFloat(BMBlackboardKeys::DistanceToTarget, Dist);

        // 设置 AI 状态为战斗
        BlackboardComp->SetValueAsEnum(BMBlackboardKeys::AIState, (uint8)EBMAIState::Combat);
    }
    else
    {
        // === 未发现玩家 / 丢失视野 ===
        
        // 如果之前有目标，现在没了，说明刚丢失 -> 清理数据
        // 注意：这里可以加入“延迟丢失”逻辑（Cooldwon），这里先做简单版
        if (BlackboardComp->GetValueAsObject(BMBlackboardKeys::TargetActor) != nullptr)
        {
            BlackboardComp->ClearValue(BMBlackboardKeys::TargetActor);
            
            // 丢失目标后，回到巡逻或待机状态
            BlackboardComp->SetValueAsEnum(BMBlackboardKeys::AIState, (uint8)EBMAIState::Patrol);
        }
    }
}