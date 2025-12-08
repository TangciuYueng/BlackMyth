#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BMCombatComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBMCombat, Log, All);

DECLARE_MULTICAST_DELEGATE(FBMOnLightAttackRequested);
DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnSkillRequested, int32 /*Slot*/);

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMCombatComponent();

    bool RequestLightAttack();
    void RequestSkill(int32 Slot);

    void ResetHitList();
    bool CanPerformAction() const;

    // 派生系统/角色监听这些事件来驱动动画/FSM（不靠蓝图）
    FBMOnLightAttackRequested OnLightAttackRequested;
    FBMOnSkillRequested OnSkillRequested;

    void SetActionLock(bool bLocked) { bActionLocked = bLocked; }

private:
    bool bActionLocked = false;
};
