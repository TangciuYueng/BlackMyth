#include "Character/Components/BMCombatComponent.h"

DEFINE_LOG_CATEGORY(LogBMCombat);

UBMCombatComponent::UBMCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

float UBMCombatComponent::GetWorldTimeSecondsSafe() const
{
    const UWorld* W = GetWorld();
    return W ? W->GetTimeSeconds() : 0.f;
}

bool UBMCombatComponent::CanPerformAction(EBMCombatAction Action) const
{
    if (bActionLocked)
    {
        if (bAllowBufferedNormalAttackWhileLocked && Action == EBMCombatAction::NormalAttack)
        {
            return true;
        }
        return false;
    }
    return true;
}

bool UBMCombatComponent::RequestAction(EBMCombatAction Action)
{
    if (CanPerformAction(Action))
    {
        OnActionRequested.Broadcast(Action);
        return true;
	}
    return false;
}

bool UBMCombatComponent::IsCooldownReady(FName Key) const
{
    if (Key.IsNone()) return true;

    const float Now = GetWorldTimeSecondsSafe();
    if (const float* End = CooldownEndTimes.Find(Key))
    {
        return Now >= *End;
    }
    return true;
}

float UBMCombatComponent::GetCooldownRemaining(FName Key) const
{
    if (Key.IsNone()) return 0.f;

    const float Now = GetWorldTimeSecondsSafe();
    if (const float* End = CooldownEndTimes.Find(Key))
    {
        return FMath::Max(0.f, *End - Now);
    }
    return 0.f;
}

void UBMCombatComponent::CommitCooldown(FName Key, float CooldownSeconds)
{
    if (Key.IsNone()) return;

    const float Cd = FMath::Max(0.f, CooldownSeconds);
    if (Cd <= 0.f) return;

    const float Now = GetWorldTimeSecondsSafe();
    CooldownEndTimes.FindOrAdd(Key) = Now + Cd;

    UE_LOG(LogBMCombat, Verbose, TEXT("[%s] CommitCooldown: %s = %.2fs"),
        *GetOwner()->GetName(), *Key.ToString(), Cd);
}

bool UBMCombatComponent::TryCommitCooldown(FName Key, float CooldownSeconds)
{
    if (!IsCooldownReady(Key))
    {
        return false;
    }
    CommitCooldown(Key, CooldownSeconds);
    return true;
}

void UBMCombatComponent::ClearCooldown(FName Key)
{
    if (!Key.IsNone())
    {
        CooldownEndTimes.Remove(Key);
    }
}

void UBMCombatComponent::ResetAllCooldowns()
{
    CooldownEndTimes.Reset();
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