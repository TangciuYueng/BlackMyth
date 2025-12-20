#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMCombatComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBMCombat, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnActionRequested, EBMCombatAction /*Action*/);

// 统一动作请求
DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnActionRequested, EBMCombatAction);

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // 关键：就是这个“无参构造”
    UBMCombatComponent();

    bool CanPerformAction() const;

    // 统一接口（推荐上层用它）
    bool RequestAction(EBMCombatAction Action);

    void ResetHitList();

    // ===== HitBox Window Context（供 AnimNotifyState 查询）=====
    void SetActiveHitBoxWindowContext(const TArray<FName>& HitBoxNames, const FBMHitBoxActivationParams& Params);
    void ClearActiveHitBoxWindowContext();
    bool GetActiveHitBoxWindowContext(TArray<FName>& OutHitBoxNames, FBMHitBoxActivationParams& OutParams) const;

    FBMOnActionRequested OnActionRequested;

    void SetActionLock(bool bLocked) { bActionLocked = bLocked; }

    // ===== 冷却系统（Per Key）=====
    bool IsCooldownReady(FName Key) const;
    float GetCooldownRemaining(FName Key) const;

    // 提交冷却：将 Key 的“下一次可用时间”推进
    void CommitCooldown(FName Key, float CooldownSeconds);

    // 原子式：如果 ready 则提交并返回 true，否则 false
    bool TryCommitCooldown(FName Key, float CooldownSeconds);

    void ClearCooldown(FName Key);
    void ResetAllCooldowns();


private:
    bool bActionLocked = false;

    UPROPERTY(Transient)
    TArray<FName> ActiveHitBoxNames;

    UPROPERTY(Transient)
    FBMHitBoxActivationParams ActiveHitBoxParams;

    // Key -> CooldownEndTime (WorldSeconds)
    UPROPERTY(Transient)
    TMap<FName, float> CooldownEndTimes;

    UPROPERTY(Transient)
    bool bHasActiveHitBoxContext = false;

    float GetWorldTimeSecondsSafe() const;
};
