#include "Character/Components/BMStatsComponent.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "UI/BMDeathWidget.h"

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

    // TrueDamage ���Է���������Ĭ�ϳԷ����������ɸ�����/������
    float Mitigated = Input;
    if (InOutInfo.DamageType != EBMDamageType::TrueDamage)
    {
        Mitigated = FMath::Max(0.f, Input - Stats.Defense);
    }

    const float OldHP = Stats.HP;
    Stats.HP = FMath::Clamp(Stats.HP - Mitigated, 0.f, Stats.MaxHP);

    const float Applied = OldHP - Stats.HP;

    // �������������Ѫ�������� UI/Ʈ��/EventBus��
    InOutInfo.DamageValue = Applied;

    // Emit HP change to UI
    if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
    {
        if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
        {
            const float Normalized = Stats.MaxHP > 0.f ? Stats.HP / Stats.MaxHP : 0.f;
            Bus->EmitPlayerHealth(Normalized);
        }
    }

    if (IsDead() && !bDeathBroadcasted)
    {
        bDeathBroadcasted = true;
        OnDeathNative.Broadcast(InOutInfo.InstigatorActor.Get());

        if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
        {
            if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
            {
                Bus->EmitPlayerDied();
            }
            if (auto* UI = GI->GetSubsystem<UBMUIManagerSubsystem>())
            {
                // Try to load a default death widget if available. You can also expose in GI if preferred.
                if (UClass* DeathClass = LoadClass<UBMDeathWidget>(nullptr, TEXT("/Game/UI/WBP_Death.WBP_Death_C")))
                {
                    UI->ShowDeath(DeathClass);
                    // Switch to UI-only input so player cannot control character and can use mouse to click
                    if (UWorld* World = GetWorld())
                    {
                        if (APlayerController* PC = World->GetFirstPlayerController())
                        {
                            FInputModeUIOnly InputMode;
                            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                            PC->SetInputMode(InputMode);
                            PC->bShowMouseCursor = true;
                        }
                    }
                }
            }
        }
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

bool UBMStatsComponent::TryConsumeMP(float Amount)
{
    if (Amount <= 0.f) return true;
    if (Stats.MP < Amount) return false;
    Stats.MP -= Amount;
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
