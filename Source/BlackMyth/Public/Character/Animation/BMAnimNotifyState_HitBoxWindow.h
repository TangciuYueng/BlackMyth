#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"

#include "Core/BMTypes.h"                  
#include "Character/Components/BMCombatComponent.h" 
#include "BMAnimNotifyState_HitBoxWindow.generated.h"

class UBMHitBoxComponent;
DECLARE_LOG_CATEGORY_EXTERN(LogBMHitBoxWindow, Log, All);
/**
 * 命中窗口 NotifyState
 *
 * 设计目标：
 * - 从 CombatComponent 的当前攻击上下文读取 HitBoxNames
 * - 负责在窗口 Begin/End 时开启/关闭 HitBox
 */
UCLASS(meta = (DisplayName = "BM HitBox Window"))
class BLACKMYTH_API UBMAnimNotifyState_HitBoxWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UBMAnimNotifyState_HitBoxWindow();

    /** 若为 true：从 CombatComponent 读取当前攻击窗口上下文 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBoxWindow")
    bool bUseCombatContext = true;

    UPROPERTY(EditAnywhere, Category = "BM|HitBoxWindow", meta = (EditCondition = "!bUseCombatContext"))
    TArray<FName> HitBoxNamesOverride;

    /** 窗口名 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBoxWindow")
    FName WindowId = TEXT("HitWindow");

    /**
     * 是否在窗口结束时强制关闭全部 HitBox
     */
    UPROPERTY(EditAnywhere, Category = "BM|HitBoxWindow")
    bool bDeactivateAllOnEnd = false;

public:
/**
 * 动画通知状态开始回调
 *
 * 在动画播放到该 NotifyState 的起始位置时触发，负责激活 HitBox 组件以检测攻击碰撞
 * 根据配置从 CombatComponent 读取攻击上下文或使用硬编码的 HitBox 名称列表
 *
 * @param MeshComp 触发通知的骨骼网格组件
 * @param Animation 当前播放的动画序列
 * @param TotalDuration 该 NotifyState 的总持续时间（秒），表示 HitBox 保持激活的时长
 * @param EventReference 动画通知事件引用，提供事件上下文和额外信息
 */
virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
    const FAnimNotifyEventReference& EventReference) override;

/**
 * 动画通知状态结束回调
 *
 * 在动画播放到该 NotifyState 的结束位置时触发，负责关闭之前激活的 HitBox 组件
 * 根据配置决定是关闭所有 HitBox 还是仅关闭本窗口启用的 HitBox
 *
 * @param MeshComp 触发通知的骨骼网格组件
 * @param Animation 当前播放的动画序列
 * @param EventReference 动画通知事件引用，提供事件上下文和额外信息
 */
virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference) override;

};
