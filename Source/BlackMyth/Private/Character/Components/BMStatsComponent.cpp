#include "Character/Components/BMStatsComponent.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "UI/BMDeathWidget.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY(LogBMStats);

UBMStatsComponent::UBMStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UBMStatsComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
    {
        if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
        {
            const float HealthNormalized = Stats.MaxHP > 0.f ? Stats.HP / Stats.MaxHP : 0.f;
            Bus->EmitPlayerHealth(HealthNormalized);

            const float StaminaNormalized = Stats.MaxStamina > 0.f ? Stats.Stamina / Stats.MaxStamina : 0.f;
            Bus->EmitPlayerStamina(StaminaNormalized);
        }
    }
}

float UBMStatsComponent::ApplyDamage(FBMDamageInfo& InOutInfo)
{
    if (IsDead())
    {
        InOutInfo.DamageValue = 0.f;
        return 0.f;
    }

    // 检查无敌状态
    if (IsInvulnerable())
    {
        UE_LOG(LogBMStats, Log, TEXT("Damage blocked by invulnerability buff"));
        InOutInfo.DamageValue = 0.f;
        return 0.f;
    }

    const float Input = InOutInfo.DamageValue;
    if (Input <= 0.f)
    {
        InOutInfo.DamageValue = 0.f;
        return 0.f;
    }

    float Mitigated = Input;
    if (InOutInfo.DamageType != EBMDamageType::TrueDamage)
    {
        // 防御公式: 减伤率 = Defense / (100 + Defense)
        const float DamageReduction = Stats.Defense / (100.f + Stats.Defense);
        Mitigated = Input * (1.f - DamageReduction);
    }

    const float OldHP = Stats.HP;
    Stats.HP = FMath::Clamp(Stats.HP - Mitigated, 0.f, Stats.MaxHP);

    const float Applied = OldHP - Stats.HP;

    InOutInfo.DamageValue = Applied;

    // Emit HP change to UI ONLY for player-controlled pawn
    if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (OwnerPawn->IsPlayerControlled())
        {
            if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
            {
                if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
                {
                    const float Normalized = Stats.MaxHP > 0.f ? Stats.HP / Stats.MaxHP : 0.f;
                    Bus->EmitPlayerHealth(Normalized);
                }
            }
        }
    }

    if (IsDead() && !bDeathBroadcasted)
    {
        bDeathBroadcasted = true;
        OnDeathNative.Broadcast(InOutInfo.InstigatorActor.Get());

        if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (OwnerPawn->IsPlayerControlled())
            {
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
        }
    }

    return Applied;
}

bool UBMStatsComponent::TryConsumeStamina(float Amount)
{
    if (Amount <= 0.f) return true;
    if (Stats.Stamina < Amount) return false;
    Stats.Stamina -= Amount;
    
    // Emit stamina change to UI
    if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
    {
        if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
        {
            const float Normalized = Stats.MaxStamina > 0.f ? Stats.Stamina / Stats.MaxStamina : 0.f;
            Bus->EmitPlayerStamina(Normalized);
        }
    }
    
    return true;
}

bool UBMStatsComponent::TryConsumeMP(float Amount)
{
    if (Amount <= 0.f) return true;
    if (Stats.MP < Amount) return false;
    Stats.MP -= Amount;
    return true;
}

void UBMStatsComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (DeltaTime <= 0.f || IsDead())
    {
        return;
    }

    // 更新增益效果计时器
    TickBuffs(DeltaTime);

    // 处理持续回血效果
    TickHealthRegen(DeltaTime);

    // 耐力恢复（应用耐力恢复加成）
    if (Stats.MaxStamina > 0.f && Stats.Stamina < Stats.MaxStamina)
    {
        const float OldStamina = Stats.Stamina;
        const float RegenMultiplier = GetStaminaRegenMultiplier();
        const float Regen = StaminaRegenPerSec * RegenMultiplier * DeltaTime;
        Stats.Stamina = FMath::Clamp(Stats.Stamina + Regen, 0.f, Stats.MaxStamina);

        if (!FMath::IsNearlyEqual(Stats.Stamina, OldStamina))
        {
            if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
            {
                if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
                {
                    const float Normalized = Stats.MaxStamina > 0.f ? Stats.Stamina / Stats.MaxStamina : 0.f;
                    Bus->EmitPlayerStamina(Normalized);
                }
            }
        }
    }
}

void UBMStatsComponent::ReviveToFull(float NewMaxHP)
{
    Stats.MaxHP = FMath::Max(1.f, NewMaxHP);
    Stats.HP = Stats.MaxHP;
    bDeathBroadcasted = false;
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

void UBMStatsComponent::InitializeFromBlock(const FBMStatBlock& In)
{
    Stats = In;
    Stats.HP = FMath::Clamp(Stats.HP, 0.f, Stats.MaxHP);
    Stats.MP = FMath::Clamp(Stats.MP, 0.f, Stats.MaxMP);
    Stats.Stamina = FMath::Clamp(Stats.Stamina, 0.f, Stats.MaxStamina);
}

// Restore HP to MaxHP and clear death flag; do not touch coins/exp/etc.
void UBMStatsComponent::Revive()
{
    bDeathBroadcasted = false;
    Stats.HP = Stats.MaxHP;
    // Notify HUD (player only)
    if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (OwnerPawn->IsPlayerControlled())
        {
            if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
            {
                if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
                {
                    const float Normalized = Stats.MaxHP > 0.f ? Stats.HP / Stats.MaxHP : 0.f;
                    Bus->EmitPlayerHealth(Normalized);
                }
            }
        }
    }
}

// ==================== 道具效果实现 ====================

void UBMStatsComponent::HealByPercent(float Percent)
{
    if (IsDead() || Percent <= 0.f)
    {
        return;
    }

    const float HealAmount = Stats.MaxHP * (Percent / 100.f);
    HealByAmount(HealAmount);
}

void UBMStatsComponent::HealByAmount(float Amount)
{
    if (IsDead() || Amount <= 0.f)
    {
        return;
    }

    const float OldHP = Stats.HP;
    const float EffectiveMaxHp = GetEffectiveMaxHp();
    Stats.HP = FMath::Clamp(Stats.HP + Amount, 0.f, EffectiveMaxHp);

    const float Healed = Stats.HP - OldHP;
    if (Healed > 0.f)
    {
        UE_LOG(LogBMStats, Log, TEXT("Healed %.1f HP (%.1f -> %.1f / %.1f)"), 
            Healed, OldHP, Stats.HP, EffectiveMaxHp);

        // 通知UI更新血量
        if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (OwnerPawn->IsPlayerControlled())
            {
                if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
                {
                    if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
                    {
                        const float Normalized = EffectiveMaxHp > 0.f ? Stats.HP / EffectiveMaxHp : 0.f;
                        Bus->EmitPlayerHealth(Normalized);
                    }
                }
            }
        }
    }
}

void UBMStatsComponent::AddBuff(EBMBuffType Type, float Value, float Duration)
{
    if (Type == EBMBuffType::None || Duration <= 0.f)
    {
        return;
    }

    // 移除同类型的旧效果（刷新机制）
    RemoveBuff(Type);

    FBMBuffInstance NewBuff(Type, Value, Duration);
    ActiveBuffs.Add(NewBuff);

    UE_LOG(LogBMStats, Log, TEXT("Added buff: Type=%d, Value=%.1f, Duration=%.1fs"), 
        static_cast<int32>(Type), Value, Duration);
}

void UBMStatsComponent::RemoveBuff(EBMBuffType Type)
{
    ActiveBuffs.RemoveAll([Type](const FBMBuffInstance& Buff)
    {
        return Buff.Type == Type;
    });
}

bool UBMStatsComponent::HasBuff(EBMBuffType Type) const
{
    return ActiveBuffs.ContainsByPredicate([Type](const FBMBuffInstance& Buff)
    {
        return Buff.Type == Type && !Buff.IsExpired();
    });
}

bool UBMStatsComponent::IsInvulnerable() const
{
    return HasBuff(EBMBuffType::Invulnerability);
}

float UBMStatsComponent::GetAttackMultiplier() const
{
    float Multiplier = 1.0f;
    for (const FBMBuffInstance& Buff : ActiveBuffs)
    {
        if (Buff.Type == EBMBuffType::AttackBoost && !Buff.IsExpired())
        {
            // Value 是百分比加成，例如 20 表示 +20%
            Multiplier += Buff.Value / 100.f;
        }
    }
    return Multiplier;
}

float UBMStatsComponent::GetStaminaRegenMultiplier() const
{
    float Multiplier = 1.0f;
    for (const FBMBuffInstance& Buff : ActiveBuffs)
    {
        if (Buff.Type == EBMBuffType::StaminaRegenBoost && !Buff.IsExpired())
        {
            // Value 是倍率加成，例如 2.0 表示 2倍恢复速度
            Multiplier *= Buff.Value;
        }
    }
    return Multiplier;
}

float UBMStatsComponent::GetEffectiveMaxHp() const
{
    float EffectiveMaxHp = Stats.MaxHP;
    
    // 查找 MaxHpBoost 增益效果
    for (const FBMBuffInstance& Buff : ActiveBuffs)
    {
        if (Buff.Type == EBMBuffType::MaxHpBoost && !Buff.IsExpired())
        {
            // Value 存储的是临时提升后的最大生命值
            EffectiveMaxHp = FMath::Max(EffectiveMaxHp, Buff.Value);
        }
    }
    
    return EffectiveMaxHp;
}

void UBMStatsComponent::TickBuffs(float DeltaTime)
{
    bool bHadExpiredBuffs = false;
    bool bMaxHpBuffExpired = false;

    for (FBMBuffInstance& Buff : ActiveBuffs)
    {
        if (!Buff.IsExpired())
        {
            Buff.RemainingTime -= DeltaTime;
            if (Buff.IsExpired())
            {
                bHadExpiredBuffs = true;
                
                // 记录是否有 MaxHpBoost 过期
                if (Buff.Type == EBMBuffType::MaxHpBoost)
                {
                    bMaxHpBuffExpired = true;
                }
                
                UE_LOG(LogBMStats, Log, TEXT("Buff expired: Type=%d"), static_cast<int32>(Buff.Type));
            }
        }
    }

    // 如果 MaxHpBoost 过期，需要调整当前HP以保持血量百分比
    if (bMaxHpBuffExpired)
    {
        const float OldEffectiveMaxHp = GetEffectiveMaxHp();
        
        // 移除已过期的增益效果（仅移除过期的）
        TArray<FBMBuffInstance> TempExpiredBuffs;
        for (const FBMBuffInstance& Buff : ActiveBuffs)
        {
            if (Buff.IsExpired() && Buff.Type == EBMBuffType::MaxHpBoost)
            {
                TempExpiredBuffs.Add(Buff);
            }
        }
        
        // 先移除过期的 MaxHpBoost
        for (const FBMBuffInstance& ExpiredBuff : TempExpiredBuffs)
        {
            ActiveBuffs.RemoveAll([&ExpiredBuff](const FBMBuffInstance& Buff)
            {
                return Buff.Type == ExpiredBuff.Type && 
                       FMath::IsNearlyEqual(Buff.Value, ExpiredBuff.Value) && 
                       Buff.IsExpired();
            });
        }
        
        // 获取新的有效最大生命值
        const float NewEffectiveMaxHp = GetEffectiveMaxHp();
        
        // 计算当前HP百分比
        const float HpPercentage = (OldEffectiveMaxHp > 0.f) ? (Stats.HP / OldEffectiveMaxHp) : 1.0f;
        
        // 根据新的最大生命值调整当前HP，保持百分比不变
        Stats.HP = FMath::Clamp(NewEffectiveMaxHp * HpPercentage, 0.f, NewEffectiveMaxHp);
        
        UE_LOG(LogBMStats, Log, TEXT("MaxHpBoost expired: MaxHP %.1f -> %.1f, HP adjusted to %.1f (%.1f%%)"), 
            OldEffectiveMaxHp, NewEffectiveMaxHp, Stats.HP, HpPercentage * 100.f);
        
        // 通知UI更新血量
        if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (OwnerPawn->IsPlayerControlled())
            {
                if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
                {
                    if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
                    {
                        const float Normalized = NewEffectiveMaxHp > 0.f ? Stats.HP / NewEffectiveMaxHp : 0.f;
                        Bus->EmitPlayerHealth(Normalized);
                    }
                }
            }
        }
    }

    // 移除其他已过期的增益效果
    if (bHadExpiredBuffs)
    {
        ActiveBuffs.RemoveAll([](const FBMBuffInstance& Buff)
        {
            return Buff.IsExpired();
        });
    }
}

void UBMStatsComponent::TickHealthRegen(float DeltaTime)
{
    // 查找持续回血效果
    for (const FBMBuffInstance& Buff : ActiveBuffs)
    {
        if (Buff.Type == EBMBuffType::HealthRegenOverTime && !Buff.IsExpired())
        {
            // Value 是在 TotalDuration 内恢复的总HP百分比
            // 每秒恢复量 = (Value / TotalDuration) * EffectiveMaxHP / 100
            if (Buff.TotalDuration > 0.f)
            {
                const float EffectiveMaxHp = GetEffectiveMaxHp();
                const float RegenPerSecond = (Buff.Value / Buff.TotalDuration) * EffectiveMaxHp / 100.f;
                const float RegenThisTick = RegenPerSecond * DeltaTime;
                
                if (RegenThisTick > 0.f && Stats.HP < EffectiveMaxHp)
                {
                    const float OldHP = Stats.HP;
                    Stats.HP = FMath::Clamp(Stats.HP + RegenThisTick, 0.f, EffectiveMaxHp);

                    // 通知UI更新血量
                    if (!FMath::IsNearlyEqual(Stats.HP, OldHP))
                    {
                        if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
                        {
                            if (OwnerPawn->IsPlayerControlled())
                            {
                                if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
                                {
                                    if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
                                    {
                                        const float Normalized = EffectiveMaxHp > 0.f ? Stats.HP / EffectiveMaxHp : 0.f;
                                        Bus->EmitPlayerHealth(Normalized);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

