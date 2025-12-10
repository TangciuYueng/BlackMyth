#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Idle.generated.h"

UCLASS()
/**
 * 玩家待机状态（Idle）
 *
 * 负责在角色静止时维持待机动画，并根据输入与运动状态触发状态切换：
 * - 检测到移动意图 -> 切换到 Move 状态
 * - 检测到离地/下落 -> 切换到 Jump 状态
 */
class BLACKMYTH_API UBMPlayerState_Idle : public UBMCharacterState
{
    GENERATED_BODY()

public:
    /**
     * 状态更新回调
     *
     * 在 Idle 状态下用于检测：
     * - 是否出现移动意图以切换到 Move
     * - 是否进入空中以切换到 Jump
     *
     * @param DeltaTime 距离上一次更新的时间间隔
     */
    virtual void OnUpdate(float DeltaTime) override;

    /**
     * 状态进入回调
     *
     * 当状态机切换到 Idle 状态时调用，通常用于：
     * - 播放/切换到待机循环动画
     * - 重置与待机相关的临时标记
     *
     * @param DeltaTime 距离上一次更新的时间间隔
     */
    virtual void OnEnter(float DeltaTime) override;
};