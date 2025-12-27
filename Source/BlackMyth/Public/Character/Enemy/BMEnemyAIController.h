#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BMEnemyAIController.generated.h"


class UBlackboardComponent;
class UBehaviorTreeComponent;

/**
 * 敌人 AI 控制器
 *
 * 管理敌人的 AI 行为，提供移动控制接口：
 * - 集成黑板和行为树组件支持复杂 AI 决策
 * - 封装移动到目标位置/Actor 的请求
 * - 提供停止移动和移动状态查询功能
 */
UCLASS()
class BLACKMYTH_API ABMEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 创建并初始化黑板和行为树组件
     */
    ABMEnemyAIController();

    /**
     * 每帧更新
     *
     * @param DeltaSeconds 帧时间间隔
     */
    virtual void Tick(float DeltaSeconds) override;

    /**
     * 拥有 Pawn 时调用
     *
     * @param InPawn 被控制的 Pawn
     */
    virtual void OnPossess(APawn* InPawn) override;

    /**
     * 请求移动到指定 Actor
     *
     * 使用导航系统移动到目标 Actor 位置
     *
     * @param Target 目标 Actor
     * @param AcceptanceRadius 接受半径，到达此距离内视为完成
     * @return 请求成功返回 true，目标无效返回 false
     */
    bool RequestMoveToActor(AActor* Target, float AcceptanceRadius);

    /**
     * 请求移动到指定位置
     *
     * 使用导航系统移动到目标世界坐标
     *
     * @param Location 目标世界坐标位置
     * @param AcceptanceRadius 接受半径，到达此距离内视为完成
     * @return 始终返回 true
     */
    bool RequestMoveToLocation(const FVector& Location, float AcceptanceRadius);

    /**
     * 请求停止移动
     *
     * 停止当前路径跟随并清除游戏焦点
     */
    void RequestStopMovement();

    /**
     * 查询移动是否激活
     *
     * @return 正在移动返回 true，否则返回 false
     */
    bool IsMoveActive() const;

public:
    /** 黑板组件，用于存储 AI 决策数据 */
    UPROPERTY(VisibleAnywhere, Category = "BM|AI")
    TObjectPtr<UBlackboardComponent> BlackboardComp;

    /** 行为树组件，用于执行 AI 行为逻辑 */
    UPROPERTY(VisibleAnywhere, Category = "BM|AI")
    TObjectPtr<UBehaviorTreeComponent> BehaviorComp;
};