#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "TimerManager.h"
#include "BMPlayerState_Attack.generated.h"

/**
 * 玩家攻击状态
 *
 * 处理玩家的所有攻击逻辑：
 * - 普通攻击连招系统
 * - 技能攻击
 * - 连段窗口和输入轮询
 * - 攻击收招阶段
 * - 攻击惯性参数控制
 * - 空中攻击限制
 * - 攻击上下文管理
 */
UCLASS()
class BLACKMYTH_API UBMPlayerState_Attack : public UBMCharacterState
{
    GENERATED_BODY()

public:
    /**
     * 进入攻击状态
     *
     * 初始化定时器、从队列取攻击动作、应用惯性设置、开始攻击
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;
    
    /**
     * 退出攻击状态
     *
     * 清理所有定时器、清除攻击上下文、解锁动作、恢复移动设置
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnExit(float DeltaTime) override;
    
    /**
     * 判断是否可以切换到目标状态
     *
     * Death/Hit 始终允许，Dodge 仅在收招阶段允许，其它需完成攻击
     *
     * @param StateName 目标状态名称
     * @return 可以切换返回 true
     */
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    /**
     * 开始连招某一段
     *
     * 设置攻击上下文、播放动画、启动连段窗口定时器和输入轮询
     *
     * @param StepIndex 连招段数索引
     */
    void StartComboStep(int32 StepIndex);
    
    /**
     * 开始技能攻击
     *
     * 设置攻击上下文、播放技能动画、提交冷却并通知UI
     *
     * @param Spec 技能攻击规格
     */
    void StartSkill(const struct FBMPlayerAttackSpec& Spec);

    /**
     * 打开连段窗口
     *
     * 允许接受下一段输入
     */
    void OpenLinkWindow();
    
    /**
     * 关闭连段窗口
     *
     * 不再接受下一段输入
     */
    void CloseLinkWindow();
    
    /**
     * 轮询连招输入
     *
     * 每帧检查是否有普通攻击输入，窗口开启时标记为有效
     */
    void PollComboInput();

    /**
     * 当前段结束回调
     *
     * 判断是否进入下一段或开始收招
     */
    void OnStepFinished();
    
    /**
     * 开始指定段的收招
     *
     * 清除攻击上下文、播放收招动画、启动收招定时器
     *
     * @param FromStepIndex 收招的段数索引
     */
    void StartRecoverForStep(int32 FromStepIndex);
    
    /**
     * 收招完成回调
     *
     * 完成攻击并切换回 Idle 状态
     */
    void OnRecoverFinished();

    /**
     * 完成攻击
     *
     * 标记完成、解锁动作
     *
     * @param bInterrupted 是否被打断
     */
    void FinishAttack(bool bInterrupted);

    /**
     * 应用攻击惯性设置
     *
     * 保存原始设置并应用攻击专用的摩擦力和减速参数
     *
     * @param Move 角色移动组件
     */
    void ApplyAttackInertiaSettings(class UCharacterMovementComponent* Move);
    
    /**
     * 恢复移动设置
     *
     * 还原攻击前的摩擦力和减速参数
     *
     * @param Move 角色移动组件
     */
    void RestoreMovementSettings(class UCharacterMovementComponent* Move);

private:
    /** 当前段结束定时器 */
    FTimerHandle TimerStepEnd;
    
    /** 连段窗口开启定时器 */
    FTimerHandle TimerWindowOpen;
    
    /** 连段窗口关闭定时器 */
    FTimerHandle TimerWindowClose;
    
    /** 输入轮询定时器 */
    FTimerHandle TimerPollInput;
    
    /** 收招结束定时器 */
    FTimerHandle TimerRecoverEnd;

    /** 攻击是否已完成 */
    bool bFinished = false;
    
    /** 是否处于收招阶段 */
    bool bRecover = false;
    
    // ===== 连招运行时状态 =====
    
    /** 是否为连招攻击 */
    bool bIsCombo = false;
    
    /** 当前连招段数索引 */
    int32 ComboIndex = -1;

    /** 连段窗口是否开启 */
    bool bLinkWindowOpen = false;
    
    /** 是否已缓存下一段输入 */
    bool bQueuedNext = false;

    // ===== 惯性参数缓存 =====
    
    /** 保存的地面摩擦力 */
    float SavedGroundFriction = 0.f;
    
    /** 保存的行走制动减速度 */
    float SavedBrakingDecelWalking = 0.f;
    
    /** 保存的制动摩擦系数 */
    float SavedBrakingFrictionFactor = 0.f;
    
    /** 保存的是否使用独立制动摩擦 */
    bool  bSavedUseSeparateBrakingFriction = false;
    
    /** 保存的制动摩擦力 */
    float SavedBrakingFriction = 0.f;
};
