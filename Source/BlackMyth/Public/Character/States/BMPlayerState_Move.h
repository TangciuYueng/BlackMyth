#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Move.generated.h"

UCLASS()
/**
 * 玩家移动状态
 *
 * 负责在角色处于地面移动时维持移动相关逻辑：
 * - 进入状态时切换移动循环动画
 * - 在失去移动意图且速度降到阈值时回到 Idle
 * - 在离地时切换到 Jump 状态
 */
class BLACKMYTH_API UBMPlayerState_Move : public UBMCharacterState
{
    GENERATED_BODY()

public:
    /**
     * 状态进入回调
     *
     * 当状态机切换到 Move 状态时调用，用于：
     * - 播放移动循环动画
     * -同步角色移动参数到当前配置
     *
     * @param DeltaTime 距离上一次 Tick 的时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 状态更新回调
     *
     * 每帧由状态机驱动，用于根据当前输入与速度判断状态切换：
     * - 若角色离地则切换到 Jump 状态
     * - 若没有移动意图且速度降到阈值，则切换回 Idle 状态
     *
     * @param DeltaTime 帧时间，用于需要时的平滑/计时逻辑。
     */
    virtual void OnUpdate(float DeltaTime) override;

private:
    /**
     * 速度停止阈值（单位：cm/s）
     *
     * 当没有移动意图且水平速度低于该阈值时，认为角色已基本停止，
     * Move 状态会切换回 Idle 状态
     */
    float StopSpeedThreshold = 3.f; 
};
