#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BMPlayerState_Attack.generated.h"

class UAnimInstance;
class UAnimMontage;

UCLASS()
/**
 * 玩家攻击状态
 *
 * 负责管理进入/退出攻击时的行为，包括：
 * - 锁定角色动作（禁止其他状态打断）
 * - 播放攻击动画或执行一次性攻击逻辑
 * - 调整移动惯性参数
 * - 在动画或计时结束后切回合适的移动/待机状态
 */
class BLACKMYTH_API UBMPlayerState_Attack : public UBMCharacterState
{
    GENERATED_BODY()

public:
    /**
     * 状态进入回调
     *
     * 在状态机切换到攻击状态时调用，用于：
     * - 锁定 Combat 动作，防止其他操作打断攻击
     * - 根据当前移动状态调整移动惯性相关参数
     * - 播放攻击动画或触发一次性攻击逻辑，并启动计时器/回调
     *
     * @param DeltaTime 距离上一次 Tick 的时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 状态退出回调
     *
     * 在攻击状态结束（无论是正常结束还是被打断）时调用，用于：
     * - 解除 Combat 动作锁
     * - 恢复进入状态前保存的移动参数（摩擦、减速度等）
     * - 清理定时器和内部标记状态
     *
     * @param DeltaTime 距离上一次 Tick 的时间间隔
     */
    virtual void OnExit(float DeltaTime) override;

    /**
     * 判断攻击状态是否允许切换到指定状态
     *
     * 通常在攻击未完成时，禁止切换到大部分其他状态，以保证攻击动作的
     * 连贯性。但可以为死亡等高优先级状态切换
     *
     * @param StateName 目标状态的名称
     * @return 若允许从当前攻击状态切换到目标状态则返回 true，否则返回 false
     */
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    /**
     * 攻击流程结束的统一处理函数
     *
     * 由计时器或动画回调触发，用于标记攻击已完成，并根据角色当前状态
     * 决定切换回 Idle 或 Move 等合适的状态
     *
     * @param bInterrupted 是否为被打断/提前终止的攻击
     */
    void FinishAttack(bool bInterrupted);
    /**
     * 进入攻击时应用移动惯性相关设置
     *
     * 在攻击状态开始时，根据当前移动组件保存原有移动参数，并设置
     * 更低的摩擦参数，使角色在攻击时保留一定滑行感
     *
     * @param Move 角色的移动组件指针
     */
    void ApplyAttackInertiaSettings(UCharacterMovementComponent* Move);

    /**
     * 恢复进入攻击前保存的移动参数
     *
     * 在攻击状态结束时调用，将移动组件的摩擦、减速度等参数恢复到原始值，
     * 避免对后续移动状态造成持续影响
     *
     * @param Move 角色的移动组件指针
     */
    void RestoreMovementSettings(UCharacterMovementComponent* Move);

private:
    /**
     * 攻击持续时间计时器句柄
     *
     * 在使用 AnimSequence + 计时器模式驱动攻击时，用于在动画预计结束时
     * 调用 FinishAttack()，实现纯 C++ 的时序控制
     */
    FTimerHandle TimerHandle;

    /**
     * 标记当前攻击是否已完成
     *
     * 用于在 CanTransitionTo 等逻辑中判断是否允许离开攻击状态，
     * 避免攻击尚未结束时被其他状态抢占
     */
    bool bFinished = false;

    /**
     * 进入攻击前保存的移动参数
     *
     * 在 ApplyAttackInertiaSettings() 中缓存角色移动组件的摩擦、减速度等关键参数，
     * 以便在攻击结束时通过 RestoreMovementSettings() 完整恢复
     */
    float SavedGroundFriction = 0.f;
    float SavedBrakingDecelWalking = 0.f;
    float SavedBrakingFrictionFactor = 0.f;
    bool  bSavedUseSeparateBrakingFriction = false;
    float SavedBrakingFriction = 0.f;
};
