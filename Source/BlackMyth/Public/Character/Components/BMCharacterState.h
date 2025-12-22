#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BMCharacterState.generated.h"

class ABMCharacterBase;

/**
 * 角色状态基类
 *
 * 状态对象以 UObject 的形式存在，由状态机组件创建并持有，
 * 通过 OnEnter/OnUpdate/OnExit 描述状态生命周期
 */
UCLASS(Abstract)
class BLACKMYTH_API UBMCharacterState : public UObject
{
    GENERATED_BODY()

public:
    /**
     * 初始化状态上下文
     *
     * 在状态被注册到状态机前/后调用，用于绑定所属角色，
     *
     * @param Owner 该状态所属的角色对象指针。
     */
    virtual void Init(ABMCharacterBase* Owner);

    /**
     * 状态进入回调
     *
     * 当状态机切换到该状态时调用。子类可在此执行：
     * - 播放动画
     * - 重置标记
     * - 加/解锁输入或动作
     *
     * @param DeltaTime 距离上一次更新的时间间隔
     */
    virtual void OnEnter(float DeltaTime) {}

    /**
     * 状态更新回调
     *
     * 由状态机每 Tick 调用。子类通常在此处理：
     * - 状态内部的条件判断与计时
     * - 触发状态切换的请求
     *
     * @param DeltaTime 帧时间间隔
     */
    virtual void OnUpdate(float DeltaTime) {}

    /**
     * 状态退出回调
     *
     * 当状态机从该状态切换离开时调用。子类可在此执行：
     * - 解除临时设置
     * - 清理定时器/事件绑定
     *
     * @param DeltaTime 距离上一次更新的时间间隔
     */
    virtual void OnExit(float DeltaTime) {}

    /**
     * 判断是否允许从当前状态切换到目标状态
     *
     * 子类可重写该方法以实现状态切换约束
     *
     * @param StateName 目标状态名称
     * @return 若允许切换则返回 true，否则返回 false
     */
    virtual bool CanTransitionTo(FName StateName) const { return true; }

    /**
     * 获取状态绑定的角色上下文
     *
     * @return 绑定的角色指针
     */
    ABMCharacterBase* GetContext() const { return Context.Get(); }

protected:
    /**
     * 状态所属角色的弱引用上下文
     */
    TWeakObjectPtr<ABMCharacterBase> Context;
};
