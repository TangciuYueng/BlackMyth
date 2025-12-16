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

bool UBMCombatComponent::RequestAction(EBMCombatAction Action)
{
    if (!CanPerformAction())
    {
        UE_LOG(LogBMCombat, Verbose, TEXT("RequestAction rejected: locked"));
        return false;
    }

    if (Action == EBMCombatAction::None)
    {
        return false;
    }

    OnActionRequested.Broadcast(Action);

    return true;
}

void UBMCombatComponent::RequestSkill(int32 Slot)
{
    if (!CanPerformAction())
    {
        UE_LOG(LogBMCombat, Verbose, TEXT("RequestSkill rejected: locked"));
        return;
    }

    OnSkillRequested.Broadcast(Slot);
}

void UBMCombatComponent::ResetHitList()
{
    // 命中列表在 HitBoxComponent 内部维护；这里只保留接口
}

void UBMCombatComponent::SetActiveHitBoxWindowContext(const TArray<FName>& HitBoxNames, const FBMHitBoxActivationParams& Params)
{
    ActiveHitBoxNames = HitBoxNames;
    ActiveHitBoxParams = Params;
    bHasActiveHitBoxContext = (ActiveHitBoxNames.Num() > 0);
}

void UBMCombatComponent::ClearActiveHitBoxWindowContext()
{
    ActiveHitBoxNames.Reset();
    ActiveHitBoxParams = FBMHitBoxActivationParams();
    bHasActiveHitBoxContext = false;
}

bool UBMCombatComponent::GetActiveHitBoxWindowContext(TArray<FName>& OutHitBoxNames, FBMHitBoxActivationParams& OutParams) const
{
    if (!bHasActiveHitBoxContext)
    {
        return false;
    }
    OutHitBoxNames = ActiveHitBoxNames;
    OutParams = ActiveHitBoxParams;
    return OutHitBoxNames.Num() > 0;
}