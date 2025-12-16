#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMCombatComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBMCombat, Log, All);

DECLARE_MULTICAST_DELEGATE(FBMOnLightAttackRequested);
DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnSkillRequested, int32 /*Slot*/);

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

    // 兼容：旧接口仍可用
    bool RequestLightAttack();
    bool RequestHeavyAttack();

    // 统一接口（推荐上层用它）
    bool RequestAction(EBMCombatAction Action);

    void RequestSkill(int32 Slot);

    void ResetHitList();

    FBMOnLightAttackRequested OnLightAttackRequested;
    FBMOnSkillRequested OnSkillRequested;
    FBMOnActionRequested OnActionRequested;

    void SetActionLock(bool bLocked) { bActionLocked = bLocked; }

private:
    bool bActionLocked = false;
};
