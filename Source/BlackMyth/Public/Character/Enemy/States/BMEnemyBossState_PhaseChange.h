#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "BMEnemyBossState_PhaseChange.generated.h"

/**
 * Boss 阶段转换状态
 *
 * 处理 Boss 从一阶段过渡到二阶段的完整流程：
 * 1. 播放死亡动画
 * 2. 短暂停顿
 * 3. 播放死亡动画倒放
 * 4. 播放蓄力动画
 * 5. 进入二阶段并返回 Idle
 *
 * 期间角色处于无敌状态且不可被打断
 */
UCLASS()
class BLACKMYTH_API UBMEnemyBossState_PhaseChange : public UBMCharacterState
{
    GENERATED_BODY()

public:
    /**
     * 进入阶段转换状态
     *
     * 锁定动作、停止移动、开启无敌并开始播放死亡动画
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 退出阶段转换状态
     *
     * 清理定时器并重置完成标记
     *
     * @param DeltaTime 帧时间间
     */
    virtual void OnExit(float DeltaTime) override;

    /**
     * 判断是否可以转换到指定状态
     *
     * 过渡期间不可被打断，仅允许转换到 Death 状态或完成后转换
     *
     * @param StateName 目标状态名称
     * @return 可以转换返回 true，否则返回 false
     */
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    /**
     * 开始死亡后停顿阶段
     *
     * 死亡动画播放完成后短暂停顿，然后进入倒放阶段
     */
    void StartHoldAfterDeath();

    /**
     * 开始死亡动画倒放阶段
     *
     * 播放死亡动画倒放以实现复活效果，完成后进入蓄力阶段
     */
    void StartDeathReverse();

    /**
     * 开始蓄力阶段
     *
     * 播放蓄力动画，完成后调用 FinishPhaseChange 进入二阶段
     */
    void StartEnergize();

    /**
     * 完成阶段转换
     *
     * 进入二阶段、恢复移动、解除无敌和动作锁，返回 Idle 状态
     */
    void FinishPhaseChange();

private:
    /** 阶段转换步骤计时器 */
    FTimerHandle StepTimer;

    /** 阶段转换是否已完成 */
    bool bFinished = false;
};
