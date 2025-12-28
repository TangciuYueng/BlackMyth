#include "Character/Components/BMCombatComponent.h"

DEFINE_LOG_CATEGORY(LogBMCombat);

/*
 * @brief Constructor of the UBMCombatComponent class
 */
UBMCombatComponent::UBMCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

/*
 * @brief Get the world time seconds safe
 * @return The world time seconds safe
 */
float UBMCombatComponent::GetWorldTimeSecondsSafe() const
{
    const UWorld* W = GetWorld();
    return W ? W->GetTimeSeconds() : 0.f;
}

/*
 * @brief Can perform action
 * @param Action The action
 * @return True if the action can be performed, false otherwise
 */
bool UBMCombatComponent::CanPerformAction(EBMCombatAction Action) const
{
    if (bActionLocked)
    {
        if (bAllowBufferedNormalAttackWhileLocked && Action == EBMCombatAction::NormalAttack)
        {
            return true;
        }
        if (Action == EBMCombatAction::Dodge)
        {
            return true;
		}
        return false;
    }
    return true;
}

/*
 * @brief Request action
 * @param Action The action
 * @return True if the action can be performed, false otherwise
 */
bool UBMCombatComponent::RequestAction(EBMCombatAction Action)
{
    if (CanPerformAction(Action))
    {
        OnActionRequested.Broadcast(Action);
        return true;
	}
    return false;
}

/*
 * @brief Is cooldown ready
 * @param Key The key
 * @return True if the cooldown is ready, false otherwise
 */
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

/*
 * @brief Get cooldown remaining
 * @param Key The key
 * @return The cooldown remaining
 */
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

/*
 * @brief Commit cooldown
 * @param Key The key
 * @param CooldownSeconds The cooldown seconds
 */
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

/*
 * @brief Try commit cooldown
 * @param Key The key
 * @param CooldownSeconds The cooldown seconds
 * @return True if the cooldown can be committed, false otherwise
 */
bool UBMCombatComponent::TryCommitCooldown(FName Key, float CooldownSeconds)
{
    if (!IsCooldownReady(Key))
    {
        return false;
    }
    CommitCooldown(Key, CooldownSeconds);
    return true;
}

/*
 * @brief Clear cooldown
 * @param Key The key
 */
void UBMCombatComponent::ClearCooldown(FName Key)
{
    if (!Key.IsNone())
    {
        CooldownEndTimes.Remove(Key);
    }
}

/*
 * @brief Reset all cooldowns
 */
void UBMCombatComponent::ResetAllCooldowns()
{
    CooldownEndTimes.Reset();
}

/*
 * @brief Reset hit list
 */
void UBMCombatComponent::ResetHitList()
{
}

/*
 * @brief Set active hit box window context
 * @param HitBoxNames The hit box names
 * @param Params The hit box activation params
 */
void UBMCombatComponent::SetActiveHitBoxWindowContext(const TArray<FName>& HitBoxNames, const FBMHitBoxActivationParams& Params)
{
    ActiveHitBoxNames = HitBoxNames;
    ActiveHitBoxParams = Params;
    bHasActiveHitBoxContext = (ActiveHitBoxNames.Num() > 0);
}

/*
 * @brief Clear active hit box window context
 */
void UBMCombatComponent::ClearActiveHitBoxWindowContext()
{
    ActiveHitBoxNames.Reset();
    ActiveHitBoxParams = FBMHitBoxActivationParams();
    bHasActiveHitBoxContext = false;
}

/*
 * @brief Get active hit box window context
 * @param OutHitBoxNames The out hit box names
 * @param OutParams The out hit box activation params
 * @return True if the active hit box window context is valid, false otherwise
 */
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