#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "Core/BMTypes.h"
#include "BMPlayerState_Jump.generated.h"

UCLASS()
/**
 * 玩家跳跃状态
 *
 * 负责管理从地面起跳到空中滞空/下落的全过程，包括：
 * - 区分主动起跳与被动掉落
 * - 在起跳阶段播放 JumpStart 动画
 * - 在空中阶段播放 FallLoop 动画
 * - 在落地后切回 Idle/Move 等其他状态
 */
class BLACKMYTH_API UBMPlayerState_Jump : public UBMCharacterState
{
    GENERATED_BODY()

public:
    /**
     * 状态进入回调
     *
     * 在状态机切换到 Jump 状态时调用，用于：
     * - 检查本次是主动起跳还是被动下落
     * - 主动起跳时播放 JumpStart 动画并触发 Jump()
     * - 被动下落或起跳动画结束后，进入 FallLoop 空中循环动画
     *
     * @param DeltaTime 距离上一次 Tick 的时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 状态退出回调
     *
     * 在从 Jump 状态切换到其他状态时调用，用于：
     * - 清理跳跃相关的计时器
     * - 重置内部标记
     * - 停止持续的 Jump 输入等
     *
     * @param DeltaTime 距离上一次 Tick 的时间间隔
     */
    virtual void OnExit(float DeltaTime) override;

    /**
     * 跳跃状态逐帧更新
     *
     * 每帧检查角色当前是否仍在空中：
     * - 若已落地，则根据移动意图切回 Idle 或 Move 状态
     * - 若是被动下落且尚未进入 FallLoop，则补充进入 FallLoop 动画
     *
     * @param DeltaTime 距离上一次 Tick 的时间间隔
     */
    virtual void OnUpdate(float DeltaTime) override;

    /**
     * 判断从 Jump 状态是否允许切换到指定状态
     *
     * 禁止在空中切换到 Attack 状态，避免空中攻击，
     * 其他状态默认允许
     *
     * @param StateName 目标状态名称
     * @return 若允许切换则返回 true，否则返回 false
     */
    virtual bool CanTransitionTo(FName StateName) const override { return StateName == BMStateNames::Attack ? false : true; }

private:
    /**
     * 在起跳动画结束后检查是否仍在空中，并在需要时切换到 FallLoop
     *
     * 通常由起跳动画播放完毕的计时器回调触发：
     * - 若角色依旧处于空中，则调用 EnterFallLoop() 播放下落循环动画
     * - 若角色已经落地，则不做处理
     */
    void StartFallLoopIfStillInAir();

    /**
     * 进入下落循环动画
     *
     * 将当前空中动作切换为 FallLoop 并标记已处于下落阶段，
     * 无论是起跳后的滞空，还是从平台边缘掉落都会进入该阶段
     */
    void EnterFallLoop();

private:
    /**
     * 从起跳动画过渡到下落循环动画的计时器句柄
     *
     * 在起跳动画播放完毕后触发回调，用于判断是否需要进入 FallLoop 状态
     */
    FTimerHandle JumpToFallHandle;

    /**
     * 是否已经进入下落循环阶段
     *
     * 用于避免重复调用 EnterFallLoop()
     */
    bool bInFallLoop = false;

    /**
     * 本次 Jump 状态是否播放过起跳动画 JumpStart
     *
     * 可用于区分两类情况：主动起跳，被动掉落
     */
    bool bDidPlayJumpStart = false;
};
