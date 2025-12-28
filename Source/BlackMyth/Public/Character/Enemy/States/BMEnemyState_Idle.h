#pragma once
#include "Character/Components/BMCharacterState.h"
#include "BMEnemyState_Idle.generated.h"

/**
 * 敌人待机状态
 *
 * 敌人的静止休息状态：
 * - 停止移动并播放待机循环动画
 * - 设置移动速度为巡逻速度
 * - 监测警戒状态和目标有效性：
 *   - 发现目标且警戒时切换到 Chase 状态
 *   - 未警戒且有巡逻路径时切换到 Patrol 状态
 */
UCLASS()
class BLACKMYTH_API UBMEnemyState_Idle : public UBMCharacterState
{
    GENERATED_BODY()
public:
    /**
     * 进入待机状态
     *
     * 停止移动、播放待机循环动画并设置移动速度为巡逻速度
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 每帧更新待机状态
     *
     * 检查警戒状态和目标有效性，决定是否：
     * - 切换到 Chase 状态（发现目标且警戒）
     * - 切换到 Patrol 状态（未警戒且有巡逻路径）
     * - 保持待机（无目标且无巡逻路径）
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnUpdate(float DeltaTime) override;
};
