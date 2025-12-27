#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMCombatComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBMCombat, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnActionRequested, EBMCombatAction /*Action*/);

// 统一动作请求
DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnActionRequested, EBMCombatAction);

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMCombatComponent();

    /**
     * 请求执行战斗动作
     *
     * 统一的战斗动作请求接口，用于处理所有类型的战斗动作
     *
     * @param Action 请求的战斗动作类型
     * @return 若动作请求被接受并广播则返回 true，若被动作锁阻止则返回 false
     */
    bool RequestAction(EBMCombatAction Action);

    void ResetHitList();

    /**
     * 设置当前激活的攻击盒窗口上下文
     *
     * 在进入攻击状态时调用，存储本次攻击使用的 HitBox 名称列表和激活参数
     *
     * @param HitBoxNames 要激活的 HitBox 名称列表
     * @param Params HitBox 激活参数
     */
    void SetActiveHitBoxWindowContext(const TArray<FName>& HitBoxNames, const FBMHitBoxActivationParams& Params);
    /**
     * 清除当前激活的攻击盒窗口上下文
     *
     * 在攻击动作结束或被打断时调用，重置所有窗口上下文数据
     */
    void ClearActiveHitBoxWindowContext();
    /**
     * 获取当前激活的攻击盒窗口上下文
     *
     * 供动画通知系统查询当前攻击需要激活哪些 HitBox 以及使用什么参数
     *
     * @param OutHitBoxNames 输出参数，返回要激活的 HitBox 名称列表
     * @param OutParams 输出参数，返回 HitBox 激活参数配置
     * @return 若存在有效的窗口上下文且 HitBox 列表非空则返回 true，否则返回 false
     */
    bool GetActiveHitBoxWindowContext(TArray<FName>& OutHitBoxNames, FBMHitBoxActivationParams& OutParams) const;

    FBMOnActionRequested OnActionRequested;

    /**
     * 设置动作锁定状态
     *
     * 控制角色是否允许执行新的战斗动作，用于在特定状态时限制输入
     *
     * @param bLocked 若为 true 则锁定动作执行，阻止大部分新动作请求；若为 false 则解除锁定
     */
    void SetActionLock(bool bLocked) { bActionLocked = bLocked; }
    /**
     * 判断是否可以执行战斗动作
     *
     * 检查当前动作锁定状态，决定指定动作是否允许执行
     *
     * @param Action 要检查的战斗动作类型，默认为 None 表示检查通用动作权限
     * @return 若允许执行该动作则返回 true，若被动作锁阻止则返回 false
     */
	bool CanPerformAction(EBMCombatAction Action = EBMCombatAction::None) const;

    /**
     * 判断指定键的冷却是否就绪
     *
     * 检查指定冷却键是否已过冷却时间，可以再次使用
     * 用于技能、闪避等需要冷却限制的动作
     *
     * @param Key 冷却键名称
     * @return 若冷却已就绪或键不存在则返回 true，若仍在冷却中则返回 false
     */
    bool IsCooldownReady(FName Key) const;
    /**
     * 获取指定键的剩余冷却时间
     *
     * 查询指定冷却键还需要多长时间才能再次使用
     *
     * @param Key 冷却键名称
     * @return 剩余冷却时间（秒），若冷却已就绪或键不存在则返回 0
     */
    float GetCooldownRemaining(FName Key) const;

    // 提交冷却
    void CommitCooldown(FName Key, float CooldownSeconds);

    /**
     * 尝试提交冷却记录
     *
     * 检查冷却是否就绪，若就绪则提交冷却并返回成功，否则直接返回失败
     *
     * @param Key 冷却键名称
     * @param CooldownSeconds 冷却时长（秒）
     * @return 若冷却就绪并成功提交则返回 true，若仍在冷却中则返回 false
     */
    bool TryCommitCooldown(FName Key, float CooldownSeconds);

    /**
     * 清除指定键的冷却记录
     *
     * 立即移除指定冷却键的记录，使其可以再次使用
     *
     * @param Key 冷却键名称，用于标识要清除的冷却记录
     */
    void ClearCooldown(FName Key);
    /**
     * 重置所有冷却记录
     *
     * 清空所有冷却键的记录，使所有技能和动作都可以立即使用
     */
    void ResetAllCooldowns();


private:
    bool bActionLocked = false;

    UPROPERTY(Transient)
    TArray<FName> ActiveHitBoxNames;

    UPROPERTY(Transient)
    FBMHitBoxActivationParams ActiveHitBoxParams;

    // Key -> CooldownEndTime (WorldSeconds)
    UPROPERTY(Transient)
    TMap<FName, float> CooldownEndTimes;

    UPROPERTY(Transient)
    bool bHasActiveHitBoxContext = false;

    bool bAllowBufferedNormalAttackWhileLocked = true;

    /**
     * 安全获取世界时间
     *
     * 获取当前世界的游戏时间，用于冷却系统的时间计算
     *
     * @return 当前世界时间（秒）
     */
    float GetWorldTimeSecondsSafe() const;
};
