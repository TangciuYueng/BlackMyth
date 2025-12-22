#include "Character/Components/BMCombatComponent.h"

DEFINE_LOG_CATEGORY(LogBMCombat);

UBMCombatComponent::UBMCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UBMCombatComponent::CanPerformAction() const
{
    return !bActionLocked;
}

bool UBMCombatComponent::RequestLightAttack()
{
    if (!CanPerformAction())
    {
        return false;
    }
    OnLightAttackRequested.Broadcast();
    return true;
}

void UBMCombatComponent::RequestSkill(int32 Slot)
{
    if (!CanPerformAction())
    {
        return;
    }
    OnSkillRequested.Broadcast(Slot);
}

void UBMCombatComponent::ResetHitList()
{
    // 具体命中列表在 HitBoxComponent 内维护；这里只留接口给 AnimEvent 调用
}
