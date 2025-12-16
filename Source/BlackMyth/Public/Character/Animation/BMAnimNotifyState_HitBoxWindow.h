#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"

#include "Core/BMTypes.h"                  // FBMDamageInfo / EBMHitReaction / 等
#include "Character/Components/BMCombatComponent.h" // 下面第2步会在 CombatComponent 里加上下文接口
#include "BMAnimNotifyState_HitBoxWindow.generated.h"

class UBMHitBoxComponent;
DECLARE_LOG_CATEGORY_EXTERN(LogBMHitBoxWindow, Log, All);
/**
 * 命中窗口 NotifyState（替换旧 Notify）
 *
 * 设计目标：
 * - 不再靠 HitBoxType 推断要开哪一个盒子
 * - 从 CombatComponent 的“当前攻击上下文”读取 HitBoxNames（一次窗口可多个）
 * - 负责在窗口 Begin/End 时开启/关闭 HitBox
 *
 * 使用方式：
 * - 在攻击动画上放置该 NotifyState，覆盖你旧的 Activate/Deactivate notify
 * - Attack 状态开始时由状态机写入 CombatComponent 的 ActiveHitBoxWindowContext
 */
UCLASS(meta = (DisplayName = "BM HitBox Window"))
class BLACKMYTH_API UBMAnimNotifyState_HitBoxWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UBMAnimNotifyState_HitBoxWindow();

    /** 若为 true：从 CombatComponent 读取当前攻击窗口上下文（推荐，工程化） */
    UPROPERTY(EditAnywhere, Category = "BM|HitBoxWindow")
    bool bUseCombatContext = true;

    /**
     * 若 bUseCombatContext=false：使用这里写死的 HitBoxNames（兜底/特殊窗口）
     * 例如同一招式不同窗口开不同盒子时可用。
     */
    UPROPERTY(EditAnywhere, Category = "BM|HitBoxWindow", meta = (EditCondition = "!bUseCombatContext"))
    TArray<FName> HitBoxNamesOverride;

    /**
     * 窗口名（主要用于调试/区分；也可写入 Params.AttackId）
     * 建议：同一动画里不同窗口用不同 WindowName。
     */
    UPROPERTY(EditAnywhere, Category = "BM|HitBoxWindow")
    FName WindowId = TEXT("HitWindow");

    /**
     * 是否在窗口结束时强制关闭全部 HitBox（更保险，但会影响“多窗口重叠”的高级用法）
     * 默认 false：只关闭本窗口启用的那些（推荐）。
     */
    UPROPERTY(EditAnywhere, Category = "BM|HitBoxWindow")
    bool bDeactivateAllOnEnd = false;

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
        const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

};
