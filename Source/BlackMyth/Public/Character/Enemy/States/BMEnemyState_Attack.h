#pragma once
#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "Core/BMTypes.h"
#include "BMEnemyState_Attack.generated.h"

/**
 * 敌人攻击状态
 *
 * 处理敌人攻击行为的完整流程：
 * - 随机选择合适的攻击招式
 * - 播放攻击动画并激活 HitBox
 * - 控制转向和移动策略
 * - 提交招式冷却
 * - 完成后返回 Chase 状态
 *
 * 攻击期间可被受击、死亡、阶段转换或闪避打断
 */
UCLASS()
class BLACKMYTH_API UBMEnemyState_Attack : public UBMCharacterState
{
    GENERATED_BODY()

public:
    /**
     * 进入攻击状态
     *
     * 停止移动、选择随机攻击、设置 HitBox 上下文、播放攻击动画并锁定动作
     * 若在空中或选择攻击失败则切换到 Chase 状态
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 退出攻击状态
     *
     * 清理 HitBox 上下文、关闭所有 HitBox、解锁动作并重置状态
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnExit(float DeltaTime) override;

    /**
     * 每帧更新攻击状态
     *
     * 攻击期间持续朝向目标
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnUpdate(float DeltaTime) override;

    /**
     * 判断是否可以转换到指定状态
     *
     * 允许被死亡、受击、阶段转换或闪避打断；完成后可转换到任意状态
     *
     * @param StateName 目标状态名称
     * @return 可以转换返回 true，否则返回 false
     */
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    /**
     * 完成攻击
     *
     * 设置完成标记并切换到 Chase 状态
     */
    void FinishAttack();

private:
    /** 攻击完成定时器句柄 */
    FTimerHandle AttackFinishHandle;

    /** 攻击是否已完成 */
    bool bFinished = false;

    /** 本次选择的攻击招式规格 */
    FBMEnemyAttackSpec ActiveAttack;
};