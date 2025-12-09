#include "Character/Components/BMStatsComponent.h"

DEFINE_LOG_CATEGORY(LogBMStats);

UBMStatsComponent::UBMStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

float UBMStatsComponent::ApplyDamage(FBMDamageInfo& InOutInfo)
{
    if (IsDead())
    {
        InOutInfo.DamageValue = 0.f;
        return 0.f;
    }

    const float Input = InOutInfo.DamageValue;
    if (Input <= 0.f)
    {
        InOutInfo.DamageValue = 0.f;
        return 0.f;
    }

    // TrueDamage 不吃防御，其它默认吃防御（你后面可改曲线/比例）
    float Mitigated = Input;
    if (InOutInfo.DamageType != EBMDamageType::TrueDamage)
    {
        Mitigated = FMath::Max(0.f, Input - Stats.Defense);
    }

    const float OldHP = Stats.HP;
    Stats.HP = FMath::Clamp(Stats.HP - Mitigated, 0.f, Stats.MaxHP);

    const float Applied = OldHP - Stats.HP;

    // 回填：最终真正扣血量（用于 UI/飘字/EventBus）
    InOutInfo.DamageValue = Applied;

    if (IsDead() && !bDeathBroadcasted)
    {
        bDeathBroadcasted = true;
        OnDeathNative.Broadcast(InOutInfo.InstigatorActor.Get());
    }

    return Applied;
}

bool UBMStatsComponent::TryConsumeStamina(float Amount)
{
    if (Amount <= 0.f) return true;
    if (Stats.Stamina < Amount) return false;
    Stats.Stamina -= Amount;
    return true;
}

void UBMStatsComponent::AddGameplayTag(FName Tag)
{
    if (!Tag.IsNone()) Tags.Add(Tag);
}

void UBMStatsComponent::RemoveGameplayTag(FName Tag)
{
    if (!Tag.IsNone()) Tags.Remove(Tag);
}

bool UBMStatsComponent::HasGameplayTag(FName Tag) const
{
    return !Tag.IsNone() && Tags.Contains(Tag);
}
