#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMStateMachineComponent.generated.h"


class UBMCharacterState;

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
/**
 * 角色状态机组件（FSM）
 *
 * 负责管理一组以 Name 为键的状态对象，并维护当前激活状态：
 * - RegisterState：注册状态实例
 * - ChangeState/ChangeStateByName：执行状态切换
 * - TickState：驱动当前状态的 OnUpdate
 *
 * 该组件仅负“状态生命周期与切换，具体状态逻辑由 UBMCharacterState 子类实现
 */
class BLACKMYTH_API UBMStateMachineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 初始化状态容器与当前状态信息
     */
    UBMStateMachineComponent();

    /**
     * 注册一个状态实例到状态机
     *
     * 状态对象通常使用 NewObject 创建
     *
     * @param Name 状态名键
     * @param State 状态实例指针，通常为 UBMCharacterState 子类。
     */
    void RegisterState(FName Name, UBMCharacterState* State);

    /**
     * 切换到指定状态实例
     *
     * 内部应按顺序触发：
     * - 当前状态 OnExit
     * - 更新 Current/CurrentStateName
     * - 新状态 OnEnter
     *
     * 建议外部优先调用 ChangeStateByName/ChangeStateById
     *
     * @param NewState 目标状态实例指针
     */
    void ChangeState(UBMCharacterState* NewState);

    /**
     * 通过状态名切换状态
     *
     * 在已注册的状态表中查找 Name 对应的状态实例并执行切换
     * 若不存在或不允许切换，则返回 false
     *
     * @param Name 目标状态名键
     * @return 切换成功返回 true；若找不到状态或切换被拒绝返回 false
     */
    bool ChangeStateByName(FName Name);

    /**
     * 通过状态枚举 ID 切换状态
     *
     * 该方法会将枚举 ID 映射为统一的状态名（BMStateNames::ToName）
     * 然后复用 ChangeStateByName 执行状态切换
     *
     * @param Id 目标状态枚举 ID
     * @return 切换成功返回 true；否则返回 false
     */
    bool ChangeStateById(EBMCharacterStateId Id)
    {
        return ChangeStateByName(BMStateNames::ToName(Id));
    }

    /**
     * 驱动当前状态更新
     *
     * 通常在 Owner 的 Tick 中调用，用于执行当前状态的 OnUpdate
     * 若当前状态为空则不执行任何逻辑
     *
     * @param DeltaSeconds 帧时间间隔
     */
    void TickState(float DeltaSeconds);

    /**
     * 获取当前激活状态的名称
     *
     * @return 当前状态名；若尚未进入任何状态则为 NAME_None
     */
    FName GetCurrentStateName() const { return CurrentStateName; }

    /**
     * 获取当前激活的状态实例
     *
     * @return 当前状态对象指针；若尚未进入任何状态则为 nullptr
     */
    UBMCharacterState* GetCurrentState() const { return Current; }



    
private:
    /**
     * 当前激活的状态对象
     *
     * 由状态机在切换时维护，TickState 将驱动该状态的 OnUpdate
     */
    UPROPERTY()
    TObjectPtr<UBMCharacterState> Current = nullptr;

    /**
     * 已注册的状态表
     *
     * Key 为状态名，Value 为状态实例。用于通过 Name/Id 查找目标状态并切换
     */
    UPROPERTY()
    TMap<FName, TObjectPtr<UBMCharacterState>> States;

    /**
     * 当前状态名缓存
     *
     * 用于快速查询当前状态的标识
     */
    FName CurrentStateName = NAME_None;
};
