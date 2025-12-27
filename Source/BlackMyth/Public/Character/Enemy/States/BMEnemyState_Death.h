#pragma once

#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "BMEnemyState_Death.generated.h"

/**
 * 敌人死亡状态
 *
 * 处理敌人死亡的完整流程：
 * - 停止移动并关闭所有 HitBox
 * - 禁用碰撞体防止进一步交互
 * - 播放死亡动画
 * - 动画结束后销毁 Actor
 *
 * 该状态不可被任何其他状态打断
 */
UCLASS()
class BLACKMYTH_API UBMEnemyState_Death : public UBMCharacterState
{
    GENERATED_BODY()
public:
    /**
     * 进入死亡状态
     *
     * 停止移动、关闭所有 HitBox、禁用碰撞并播放死亡动画
     * 动画结束后延迟短暂时间销毁 Actor
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 判断是否可以转换到指定状态
     *
     * 死亡状态为终结状态，不允许转换到任何其他状态
     *
     * @param StateName 目标状态名称
     * @return 始终返回 false
     */
    virtual bool CanTransitionTo(FName StateName) const override { return false; }

private:
    /**
     * 完成死亡流程
     *
     * 设置 Actor 生命周期为 0.1 秒后销毁
     */
    void FinishDeath();

private:
    /** 死亡完成定时器句柄 */
    FTimerHandle DeathFinishHandle;
};
