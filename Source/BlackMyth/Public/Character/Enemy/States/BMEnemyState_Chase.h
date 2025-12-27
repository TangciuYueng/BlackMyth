#pragma once
#include "Character/Components/BMCharacterState.h"
#include "BMEnemyState_Chase.generated.h"

/**
 * 敌人追击状态
 *
 * 追击玩家目标直到进入攻击范围或失去目标：
 * - 设置移动速度为追击速度
 * - 持续向目标移动并面向目标
 * - 进入攻击范围且满足攻击条件时切换到 Attack 状态
 * - 失去目标或警戒解除时返回 Patrol 或 Idle 状态
 * - 根据移动速度动态切换 Run/Idle 动画
 */
UCLASS()
class BLACKMYTH_API UBMEnemyState_Chase : public UBMCharacterState
{
    GENERATED_BODY()
public:
    /**
     * 进入追击状态
     *
     * 播放奔跑循环动画并设置移动速度为追击速度
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;

    /**
     * 每帧更新追击状态
     *
     * 检查目标有效性、距离和攻击条件，决定是否：
     * - 切换到 Attack 状态（进入攻击范围且可攻击）
     * - 返回 Patrol/Idle 状态（失去目标）
     * - 继续追击并根据速度切换动画
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnUpdate(float DeltaTime) override;
};