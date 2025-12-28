#pragma once
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Hit.generated.h"

/**
 * 玩家受击状态
 *
 * 处理玩家受击逻辑：
 * - 锁定动作（受击期间无法操作）
 * - 播放受击动画（根据伤害类型选择轻击或重击动画）
 * - 受击硬直时间控制
 * - 自动恢复到 Idle 或 Move 状态
 * - 可被 Death 状态打断
 */
UCLASS()
class BLACKMYTH_API UBMPlayerState_Hit : public UBMCharacterState
{
    GENERATED_BODY()
public:
    /**
     * 进入受击状态
     *
     * 锁定动作、根据伤害信息播放受击动画、设置硬直时间定时器
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float) override;
    
    /**
     * 退出受击状态
     *
     * 清理定时器、解锁动作
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnExit(float) override;
    
    /**
     * 判断是否可以切换到目标状态
     *
     * Death 始终允许，其它需完成受击硬直
     *
     * @param StateName 目标状态名称
     * @return 可以切换返回 true
     */
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    /** 受击硬直定时器 */
    FTimerHandle TimerHandle;
    
    /** 受击是否已完成 */
    bool bFinished = false;
};