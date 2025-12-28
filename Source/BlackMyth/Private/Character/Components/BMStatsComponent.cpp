#include "Character/Components/BMStatsComponent.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "UI/BMDeathWidget.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY(LogBMStats);

/*
 * @brief Constructor of the UBMStatsComponent class
 */
UBMStatsComponent::UBMStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

/*
 * @brief Begin play, it initializes the stats
 */
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

/*
 * @brief Apply damage, it applies the damage to the stats
 * @param InOutInfo The incoming damage info
 * @return The applied damage
 */
float UBMStatsComponent::ApplyDamage(FBMDamageInfo& InOutInfo)
{
    if (IsDead())
    {
        InOutInfo.DamageValue = 0.f;
        return 0.f;
    }

    // ����޵�״̬
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
        // ������ʽ: ������ = Defense / (100 + Defense)
        const float DamageReduction = Stats.Defense / (100.f + Stats.Defense);
        Mitigated = Input * (1.f - DamageReduction);
    }

    const float OldHP = Stats.HP;
    Stats.HP = FMath::Clamp(Stats.HP - Mitigated, 0.f, Stats.MaxHP);

    const float Applied = OldHP - Stats.HP;

    InOutInfo.DamageValue = Applied;

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
        else
        {
            if (OwnerPawn->ActorHasTag(TEXT("Boss")))
            {
                if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
                {
                    if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
                    {
                        const float Normalized = Stats.MaxHP > 0.f ? Stats.HP / Stats.MaxHP : 0.f;
                        Bus->EmitBossHealth(Normalized);
                    }
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

/*
 * @brief Try consume stamina, it tries to consume the stamina
 * @param Amount The amount of stamina to consume
 * @return True if the stamina is consumed, false otherwise
 */
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

/*
 * @brief Try consume MP, it tries to consume the MP
 * @param Amount The amount of MP to consume
 * @return True if the MP is consumed, false otherwise
 */
bool UBMStatsComponent::TryConsumeMP(float Amount)
{
    if (Amount <= 0.f) return true;
    if (Stats.MP < Amount) return false;
    Stats.MP -= Amount;
    return true;
}

/*
 * @brief Tick component, it ticks the component
 * @param DeltaTime The delta time
 * @param TickType The tick type
 * @param ThisTickFunction The tick function
 */
void UBMStatsComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (DeltaTime <= 0.f || IsDead())
    {
        return;
    }

    // ��������Ч����ʱ��
    TickBuffs(DeltaTime);

    // ����������ѪЧ��
    TickHealthRegen(DeltaTime);

    // �����ָ�
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

/*
 * @brief Revive to full, it revives the stats to full
 * @param NewMaxHP The new max HP
 */
void UBMStatsComponent::ReviveToFull(float NewMaxHP)
{
    Stats.MaxHP = FMath::Max(1.f, NewMaxHP);
    Stats.HP = Stats.MaxHP;
    bDeathBroadcasted = false;
}

/*
 * @brief Add gameplay tag, it adds the gameplay tag to the stats
 * @param Tag The tag to add
 */
void UBMStatsComponent::AddGameplayTag(FName Tag)
{
    if (!Tag.IsNone()) Tags.Add(Tag);
}

/*
 * @brief Remove gameplay tag, it removes the gameplay tag from the stats
 * @param Tag The tag to remove
 */
void UBMStatsComponent::RemoveGameplayTag(FName Tag)
{
    if (!Tag.IsNone()) Tags.Remove(Tag);
}

/*
 * @brief Has gameplay tag, it checks if the stats has the gameplay tag
 * @param Tag The tag to check
 * @return True if the stats has the gameplay tag, false otherwise
 */
bool UBMStatsComponent::HasGameplayTag(FName Tag) const
{
    return !Tag.IsNone() && Tags.Contains(Tag);
}

/*
 * @brief Initialize from block, it initializes the stats from the block
 * @param In The block to initialize from
 */
void UBMStatsComponent::InitializeFromBlock(const FBMStatBlock& In)
{
    Stats = In;
    Stats.HP = FMath::Clamp(Stats.HP, 0.f, Stats.MaxHP);
    Stats.MP = FMath::Clamp(Stats.MP, 0.f, Stats.MaxMP);
    Stats.Stamina = FMath::Clamp(Stats.Stamina, 0.f, Stats.MaxStamina);
}

/*
 * @brief Revive, it revives the stats
 */
void UBMStatsComponent::Revive()
{
    bDeathBroadcasted = false;
    Stats.HP = Stats.MaxHP;
    // Notify HUD 
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

// ==================== ����Ч��ʵ�� ====================

/*
 * @brief Heal by percent, it heals the stats by percent
 * @param Percent The percent to heal
 */
void UBMStatsComponent::HealByPercent(float Percent)
{
    if (IsDead() || Percent <= 0.f)
    {
        return;
    }

    const float HealAmount = Stats.MaxHP * (Percent / 100.f);
    HealByAmount(HealAmount);
}

/*
 * @brief Heal by amount, it heals the stats by amount
 * @param Amount The amount to heal
 */
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

        // ֪ͨUI����Ѫ��
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

/*
 * @brief Add buff, it adds the buff to the stats
 * @param Type The type of the buff
 * @param Value The value of the buff
 * @param Duration The duration of the buff
 */
void UBMStatsComponent::AddBuff(EBMBuffType Type, float Value, float Duration)
{
    if (Type == EBMBuffType::None || Duration <= 0.f)
    {
        return;
    }

    // �Ƴ�ͬ���͵ľ�Ч��
    RemoveBuff(Type);

    FBMBuffInstance NewBuff(Type, Value, Duration);
    ActiveBuffs.Add(NewBuff);

    UE_LOG(LogBMStats, Log, TEXT("Added buff: Type=%d, Value=%.1f, Duration=%.1fs"), 
        static_cast<int32>(Type), Value, Duration);
}

/*
 * @brief Remove buff, it removes the buff from the stats
 * @param Type The type of the buff
 */
void UBMStatsComponent::RemoveBuff(EBMBuffType Type)
{
    ActiveBuffs.RemoveAll([Type](const FBMBuffInstance& Buff)
    {
        return Buff.Type == Type;
    });
}

/*
 * @brief Has buff, it checks if the stats has the buff
 * @param Type The type of the buff
 * @return True if the stats has the buff, false otherwise
 */
bool UBMStatsComponent::HasBuff(EBMBuffType Type) const
{
    return ActiveBuffs.ContainsByPredicate([Type](const FBMBuffInstance& Buff)
    {
        return Buff.Type == Type && !Buff.IsExpired();
    });
}

/*
 * @brief Is invulnerable, it checks if the stats is invulnerable
 * @return True if the stats is invulnerable, false otherwise
 */
bool UBMStatsComponent::IsInvulnerable() const
{
    return HasBuff(EBMBuffType::Invulnerability);
}

/*
 * @brief Get attack multiplier, it gets the attack multiplier
 * @return The attack multiplier
 */
float UBMStatsComponent::GetAttackMultiplier() const
{
    float Multiplier = 1.0f;
    for (const FBMBuffInstance& Buff : ActiveBuffs)
    {
        if (Buff.Type == EBMBuffType::AttackBoost && !Buff.IsExpired())
        {
            // �ٷֱȼӳ�
            Multiplier += Buff.Value / 100.f;
        }
    }
    return Multiplier;
}

/*
 * @brief Get stamina regen multiplier, it gets the stamina regen multiplier
 * @return The stamina regen multiplier
 */
float UBMStatsComponent::GetStaminaRegenMultiplier() const
{
    float Multiplier = 1.0f;
    for (const FBMBuffInstance& Buff : ActiveBuffs)
    {
        if (Buff.Type == EBMBuffType::StaminaRegenBoost && !Buff.IsExpired())
        {
            // ���ʼӳ�
            Multiplier *= Buff.Value;
        }
    }
    return Multiplier;
}

/*
 * @brief Get effective max HP, it gets the effective max HP
 * @return The effective max HP
 */
float UBMStatsComponent::GetEffectiveMaxHp() const
{
    float EffectiveMaxHp = Stats.MaxHP;
    
    // ���� MaxHpBoost ����Ч��
    for (const FBMBuffInstance& Buff : ActiveBuffs)
    {
        if (Buff.Type == EBMBuffType::MaxHpBoost && !Buff.IsExpired())
        {
            // Value �洢������ʱ��������������ֵ
            EffectiveMaxHp = FMath::Max(EffectiveMaxHp, Buff.Value);
        }
    }
    
    return EffectiveMaxHp;
}

/*
 * @brief Tick buffs, it ticks the buffs
 * @param DeltaTime The delta time
 */
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
                
                // ��¼�Ƿ��� MaxHpBoost ����
                if (Buff.Type == EBMBuffType::MaxHpBoost)
                {
                    bMaxHpBuffExpired = true;
                }
                
                UE_LOG(LogBMStats, Log, TEXT("Buff expired: Type=%d"), static_cast<int32>(Buff.Type));
            }
        }
    }

    // ��� MaxHpBoost ���ڣ���Ҫ������ǰHP�Ա���Ѫ���ٷֱ�
    if (bMaxHpBuffExpired)
    {
        const float OldEffectiveMaxHp = GetEffectiveMaxHp();
        
        // �Ƴ��ѹ��ڵ�����Ч��
        TArray<FBMBuffInstance> TempExpiredBuffs;
        for (const FBMBuffInstance& Buff : ActiveBuffs)
        {
            if (Buff.IsExpired() && Buff.Type == EBMBuffType::MaxHpBoost)
            {
                TempExpiredBuffs.Add(Buff);
            }
        }
        
        // ���Ƴ����ڵ� MaxHpBoost
        for (const FBMBuffInstance& ExpiredBuff : TempExpiredBuffs)
        {
            ActiveBuffs.RemoveAll([&ExpiredBuff](const FBMBuffInstance& Buff)
            {
                return Buff.Type == ExpiredBuff.Type && 
                       FMath::IsNearlyEqual(Buff.Value, ExpiredBuff.Value) && 
                       Buff.IsExpired();
            });
        }
        
        // ��ȡ�µ���Ч�������ֵ
        const float NewEffectiveMaxHp = GetEffectiveMaxHp();
        
        // ���㵱ǰHP�ٷֱ�
        const float HpPercentage = (OldEffectiveMaxHp > 0.f) ? (Stats.HP / OldEffectiveMaxHp) : 1.0f;
        
        // �����µ��������ֵ������ǰHP�����ְٷֱȲ���
        Stats.HP = FMath::Clamp(NewEffectiveMaxHp * HpPercentage, 0.f, NewEffectiveMaxHp);
        
        UE_LOG(LogBMStats, Log, TEXT("MaxHpBoost expired: MaxHP %.1f -> %.1f, HP adjusted to %.1f (%.1f%%)"), 
            OldEffectiveMaxHp, NewEffectiveMaxHp, Stats.HP, HpPercentage * 100.f);
        
        // ֪ͨUI����Ѫ��
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

    // �Ƴ������ѹ��ڵ�����Ч��
    if (bHadExpiredBuffs)
    {
        ActiveBuffs.RemoveAll([](const FBMBuffInstance& Buff)
        {
            return Buff.IsExpired();
        });
    }
}

/*
 * @brief Tick health regen, it ticks the health regen
 * @param DeltaTime The delta time
 */
void UBMStatsComponent::TickHealthRegen(float DeltaTime)
{
    // ���ҳ�����ѪЧ��
    for (const FBMBuffInstance& Buff : ActiveBuffs)
    {
        if (Buff.Type == EBMBuffType::HealthRegenOverTime && !Buff.IsExpired())
        {
            // Value ���� TotalDuration �ڻָ�����HP�ٷֱ�
            // ÿ��ָ��� = (Value / TotalDuration) * EffectiveMaxHP / 100
            if (Buff.TotalDuration > 0.f)
            {
                const float EffectiveMaxHp = GetEffectiveMaxHp();
                const float RegenPerSecond = (Buff.Value / Buff.TotalDuration) * EffectiveMaxHp / 100.f;
                const float RegenThisTick = RegenPerSecond * DeltaTime;
                
                if (RegenThisTick > 0.f && Stats.HP < EffectiveMaxHp)
                {
                    const float OldHP = Stats.HP;
                    Stats.HP = FMath::Clamp(Stats.HP + RegenThisTick, 0.f, EffectiveMaxHp);

                    // ֪ͨUI����Ѫ��
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

