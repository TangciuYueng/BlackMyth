#pragma once
#include "Character/Components/BMCharacterState.h"
#include "BMEnemyState_Patrol.generated.h"

/**
 * 敌人巡逻状态
 *
 * 在指定半径内随机选择目标点并移动的巡逻行为：
 * - 设置移动速度为巡逻速度
 * - 定期在巡逻半径内随机选择新的目标点
 * - 根据移动速度动态切换 Walk/Idle 动画
 * - 发现目标且警戒时切换到 Chase 状态
 */
UCLASS()
class BLACKMYTH_API UBMEnemyState_Patrol : public UBMCharacterState
{
    GENERATED_BODY()
public:
    /**
     * 进入巡逻状态
     *
     * 播放行走循环动画、设置移动速度为巡逻速度并立即触发选点
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 每帧更新巡逻状态
     *
     * 检查警戒状态、根据速度切换动画、定期重新选择巡逻目标点
     * 发现目标且警戒时切换到 Chase 状态
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnUpdate(float DeltaTime) override;

private:
    /** 重新寻路累积时间 */
    float RepathAccum = 0.f;

    /** 当前巡逻目标位置 */
    FVector PatrolDest = FVector::ZeroVector;
};