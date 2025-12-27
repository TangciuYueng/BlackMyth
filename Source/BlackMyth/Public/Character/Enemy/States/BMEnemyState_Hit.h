#pragma once

#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "BMEnemyState_Hit.generated.h"

/**
 * 敌人受击状态
 *
 * 处理敌人受到攻击时的反应：
 * - 停止移动
 * - 播放受击动画
 * - 动画结束后根据警戒状态决定下一个状态
 *
 * 受击期间仅可被死亡状态打断
 */
UCLASS()
class BLACKMYTH_API UBMEnemyState_Hit : public UBMCharacterState
{
    GENERATED_BODY()
public:
    /**
     * 进入受击状态
     *
     * 停止移动并播放受击动画，根据上次伤害信息选择合适的受击反应
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 退出受击状态
     *
     * 清理定时器并重置完成标记
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnExit(float DeltaTime) override;

    /**
     * 判断是否可以转换到指定状态
     *
     * 受击状态仅允许被死亡状态打断，完成后可转换到任意状态
     *
     * @param StateName 目标状态名称
     * @return 可以转换返回 true，否则返回 false
     */
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    /**
     * 完成受击流程
     *
     * 根据警戒状态和目标有效性决定切换到 Chase、Patrol 或 Idle 状态
     */
    void FinishHit();

private:
    /** 受击完成定时器句柄 */
    FTimerHandle HitFinishHandle;

    /** 受击是否已完成 */
    bool bFinished = false;
};
