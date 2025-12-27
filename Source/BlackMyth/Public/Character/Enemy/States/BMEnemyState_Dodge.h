#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "BMEnemyState_Dodge.generated.h"

/**
 * 敌人闪避状态
 *
 * 处理敌人闪避行为的完整流程：
 * - 关闭所有 HurtBox 获得无敌效果
 * - 根据攻击来源方向决定闪避方向
 * - 分步推进角色位移，带悬崖边缘检测
 * - 播放闪避动画
 * - 完成后恢复 HurtBox 并返回 Chase 或 Idle 状态
 *
 * 闪避期间不可被打断，仅允许死亡状态强制中断
 */
UCLASS()
class BLACKMYTH_API UBMEnemyState_Dodge : public UBMCharacterState
{
	GENERATED_BODY()
public:
    /**
     * 进入闪避状态
     *
     * 停止移动、锁定动作、关闭 HurtBox、计算闪避方向并播放动画
     * 启动分步位移计时器以平滑推进角色移动
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 退出闪避状态
     *
     * 清理定时器、恢复 HurtBox、解锁动作并重置状态
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnExit(float DeltaTime) override;

    /**
     * 判断是否可以转换到指定状态
     *
     * 闪避状态不可被打断，仅允许死亡状态强制中断；完成后可转换到任意状态
     *
     * @param StateName 目标状态名称
     * @return 可以转换返回 true，否则返回 false
     */
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    /**
     * 分步推进闪避位移
     *
     * 每帧计算并应用位移增量，带悬崖边缘检测防止掉落
     * 当完成目标距离或遇到障碍时停止推进
     */
    void StepDodge();

    /**
     * 检测指定位置是否有可行走的地面
     *
     * 用于悬崖边缘检测，防止闪避时掉落
     *
     * @param WorldPos 要检测的世界坐标位置
     * @param Char 角色实例，用于获取碰撞查询参数
     * @return 有可行走地面返回 true，否则返回 false
     */
    bool HasWalkableFloorAt(const FVector& WorldPos, const class ACharacter* Char) const;

    /**
     * 完成闪避流程
     *
     * 设置完成标记并根据警戒状态切换到 Chase 或 Idle 状态
     */
    void FinishDodge();

private:
    /** 闪避完成定时器句柄 */
    FTimerHandle TimerHandleFinish;

    /** 分步位移定时器句柄 */
    FTimerHandle TimerHandleStep;

    /** 闪避是否已完成 */
    bool bFinished = false;

    /** 锁定的闪避方向（归一化向量） */
    FVector LockedDir = FVector::BackwardVector;

    /** 目标闪避总距离 */
    float TotalDistance = 0.f;

    /** 已移动距离 */
    float TraveledDistance = 0.f;

    /** 闪避动画窗口持续时间 */
    float WindowDuration = 0.f;

    /** 上次分步推进的时间戳 */
    float LastStepTime = 0.f;

    /** 悬崖检测向上探测距离 */
    float LedgeProbeUp = 50.f;

    /** 悬崖检测向下探测距离 */
    float LedgeProbeDown = 160.f;
	
};
