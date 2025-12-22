#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BMCombatComponent.generated.h"

/** 战斗系统日志类别。用于输出攻击请求、技能请求、命中列表重置等调试信息 */
DECLARE_LOG_CATEGORY_EXTERN(LogBMCombat, Log, All);

/**
 * 轻攻击请求事件
 *
 * 当 RequestLightAttack() 通过校验后触发，供角色/FSM/动画驱动系统监听
 */
DECLARE_MULTICAST_DELEGATE(FBMOnLightAttackRequested);

/**
 * 技能请求事件
 *
 * 当 RequestSkill() 通过校验后触发，监听者可根据 Slot 决定施放哪个技能、
 * 切换哪个技能状态或触发哪段动画
 *
 * @param Slot 技能槽位索引
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnSkillRequested, int32 /*Slot*/);

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
/**
 * 战斗组件（Combat）
 *
 * 作为“动作请求入口”，负责接收角色输入或 AI 决策产生的战斗意图，
 * 并进行最基础的合法性校验，随后通过委托广播事件，
 * 由上层系统驱动具体表现与逻辑。
 */
class BLACKMYTH_API UBMCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 初始化战斗组件的内部状态，并可在此处设置 Tick 等基础属性
     */
    UBMCombatComponent();

    /**
     * 请求执行轻攻击
     *
     * 会根据当前状态进行基础校验
     * 校验通过后广播 OnLightAttackRequested 事件
     *
     * @return 若请求被接受并成功广播事件返回 true；若被拒绝返回 false
     */
    bool RequestLightAttack();

    /**
     * 请求施放指定技能槽位的技能
     *
     * 会进行基础校验，通过后广播 OnSkillRequested
     *
     * @param Slot 技能槽位索引
     */
    void RequestSkill(int32 Slot);

    /**
     * 重置命中列表/命中缓存
     *
     * 用于在一次攻击动作开始或结束时清空已命中的目标集合，
     * 防止同一段攻击判定对同一目标重复结算
     */
    void ResetHitList();

    /**
     * 判断当前是否允许执行动作
     *
     * 通常被角色输入处理、状态机或技能系统调用，用于统一判断是否处于动作锁定状态
     *
     * @return 若未被动作锁限制则返回 true，否则返回 false
     */
    bool CanPerformAction() const;

    /**
     * 轻攻击请求事件实例
     *
     * 外部系统可绑定监听该事件，以在轻攻击请求通过时驱动进入 Attack 状态、
     * 播放攻击动画或触发判定逻辑
     */
    FBMOnLightAttackRequested OnLightAttackRequested;

    /**
     * 技能请求事件实例
     *
     * 外部系统可绑定监听该事件，以在技能请求通过时驱动进入 SkillCast 状态、
     * 扣除资源并执行技能逻辑
     */
    FBMOnSkillRequested OnSkillRequested;

    /**
     * 设置动作锁定状态
     *
     * 当角色处于攻击、受击、施法等不可被输入打断的阶段时，
     * 可通过此接口锁定动作请求；解锁后恢复正常输入响应
     *
     * @param bLocked 是否锁定动作。true 为锁定，false 为解锁
     */
    void SetActionLock(bool bLocked) { bActionLocked = bLocked; }

private:
    /**
     * 动作锁定标记
     *
     * 为 true 时，RequestLightAttack/RequestSkill 等动作请求将被拒绝，
     * 用于避免动作被频繁打断或在不允许的时机触发
     */
    bool bActionLocked = false;
};
