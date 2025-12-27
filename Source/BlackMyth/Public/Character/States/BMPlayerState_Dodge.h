// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Dodge.generated.h"

class UCharacterMovementComponent;

/**
 * 玩家闪避状态
 *
 * 处理玩家闪避逻辑：
 * - 消耗耐力和冷却检查
 * - 无敌帧
 * - 锁定闪避方向和朝向
 * - 分步位移控制
 * - 平台边缘保护
 * - 障碍物碰撞检测
 * - 移动参数临时调整
 * - 动作锁定
 */
UCLASS()
class BLACKMYTH_API UBMPlayerState_Dodge : public UBMCharacterState
{
	GENERATED_BODY()
public:
    /**
     * 进入闪避状态
     *
     * 检查耐力和冷却、禁用 HurtBox、锁定方向、设置移动参数、启动位移步进
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;
    
    /**
     * 退出闪避状态
     *
     * 清理定时器、恢复 HurtBox、解锁动作、恢复移动参数
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnExit(float DeltaTime) override;
    
    /**
     * 判断是否可以切换到目标状态
     *
     * Death 始终允许，其它需完成闪避
     *
     * @param StateName 目标状态名称
     * @return 可以切换返回 true
     */
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    /**
     * 完成闪避
     *
     * 标记完成并切换回 Idle 或 Move 状态
     */
    void FinishDodge();
    
    /**
     * 步进位移
     *
     * 每帧计算并执行一小段位移，检测边缘和障碍物
     */
    void StepDodge();
    
    /**
     * 检查指定位置是否有可行走的地面
     *
     * 用于边缘保护，防止闪避掉下平台
     *
     * @param WorldPos 世界空间位置
     * @param Char 角色指针
     * @return 有可行走地面返回 true
     */
    bool HasWalkableFloorAt(const FVector& WorldPos, const class ACharacter* Char) const;

private:
    /** 闪避完成定时器 */
    FTimerHandle TimerHandleFinish;
    
    /** 位移步进定时器*/
    FTimerHandle TimerHandleStep;

    /** 闪避是否已完成 */
    bool bFinished = false;

    /** 锁定的闪避方向*/
    FVector LockedDir = FVector::ForwardVector;

    /** 总位移距离 */
    float TotalDistance = 0.f;
    
    /** 已移动距离 */
    float TraveledDistance = 0.f;
    
    /** 闪避动画时长 */
    float WindowDuration = 0.f;

    /** 上次步进时间 */
    float LastStepTime = 0.f;

    // ===== 边缘保护参数 =====
    
    /** 向上探测距离*/
    float LedgeProbeUp = 50.f;
    
    /** 向下探测距离 */
    float LedgeProbeDown = 150.f;

    // ===== 移动参数缓存 =====
    
    /** 保存的朝向移动方向旋转标志 */
    bool bSavedOrientToMovement = true;
    
    /** 保存的最大行走速度 */
    float SavedMaxWalkSpeed = 0.f;
};
