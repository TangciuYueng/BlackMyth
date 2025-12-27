#pragma once
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Death.generated.h"

/**
 * 玩家死亡状态
 *
 * 处理玩家死亡逻辑：
 * - 锁定所有动作
 * - 禁用角色移动
 * - 禁用碰撞
 * - 播放死亡动画
 * - 不允许切换到任何其他状态
 */
UCLASS()
class BLACKMYTH_API UBMPlayerState_Death : public UBMCharacterState
{
    GENERATED_BODY()
public:
    /**
     * 进入死亡状态
     *
     * 锁定动作、禁用移动和碰撞、播放死亡动画
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnEnter(float) override;
    
    /**
     * 判断是否可以切换到目标状态
     *
     * 死亡状态是终结状态，不允许切换到任何状态
     *
     * @param StateName 目标状态名称
     * @return 始终返回 false
     */
    virtual bool CanTransitionTo(FName) const override { return false; }
};
