#include "Character/Components/BMExperienceComponent.h"
#include "Character/Components/BMStatsComponent.h"
#include "Core/BMDataSubsystem.h"
#include "Data/BMPlayerGrowthData.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogBMExperience);

UBMExperienceComponent::UBMExperienceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // 初始化默认值
    Level = 1;
    CurrentXP = 0.0f;
    SkillPoints = 0;
    AttributePoints = 0;
    
    // 默认配置
    BaseXPRequired = 100.0f;
    XPGrowthMultiplier = 1.15f;
    SkillPointsPerLevel = 1;
    AttributePointsPerLevel = 1;
    MaxLevel = 0; // 默认无限制
}

void UBMExperienceComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 广播初始状态
    const float MaxXP = GetMaxXPForNextLevel();
    const float Percent = GetExpPercent();
    OnXPChangedNative.Broadcast(CurrentXP, MaxXP, Percent);
    OnSkillPointChangedNative.Broadcast(SkillPoints);
    OnAttributePointChangedNative.Broadcast(AttributePoints);
    
    // 发送到 EventBus 供 UI 使用
    EmitXPChangedToEventBus(CurrentXP, MaxXP, Percent);
    EmitSkillPointsChangedToEventBus(SkillPoints);
    EmitAttributePointsChangedToEventBus(AttributePoints);
}

int32 UBMExperienceComponent::AddXP(float Amount)
{
    if (Amount <= 0.0f)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("AddXP: Attempted to add non-positive XP amount: %f"), Amount);
        return 0;
    }

    // 检查是否达到最大等级
    if (MaxLevel > 0 && Level >= MaxLevel)
    {
        UE_LOG(LogBMExperience, Verbose, TEXT("AddXP: Player is at max level %d, XP gain ignored"), MaxLevel);
        return 0;
    }

    CurrentXP += Amount;
    UE_LOG(LogBMExperience, Log, TEXT("AddXP: Added %f XP. Current: %f / %f"), 
        Amount, CurrentXP, GetMaxXPForNextLevel());

    // 检查是否可以升级（可以多次升级）
    int32 LevelsGained = 0;
    while (CheckLevelUp() > 0)
    {
        LevelsGained++;
    }

    // 广播经验值变化事件
    const float MaxXP = GetMaxXPForNextLevel();
    const float Percent = GetExpPercent();
    OnXPChangedNative.Broadcast(CurrentXP, MaxXP, Percent);
    
    // 发送到 EventBus 供 UI 使用
    EmitXPChangedToEventBus(CurrentXP, MaxXP, Percent);

    return LevelsGained;
}

int32 UBMExperienceComponent::CheckLevelUp()
{
    // 检查是否达到最大等级
    if (MaxLevel > 0 && Level >= MaxLevel)
    {
        return 0;
    }

    const float RequiredXP = GetMaxXPForNextLevel();
    
    if (CurrentXP >= RequiredXP)
    {
        return PerformLevelUp() ? 1 : 0;
    }

    return 0;
}

bool UBMExperienceComponent::PerformLevelUp()
{
    if (MaxLevel > 0 && Level >= MaxLevel)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("PerformLevelUp: Already at max level %d"), MaxLevel);
        return false;
    }

    const int32 OldLevel = Level;
    Level++;
    
    // 扣除当前等级所需的经验值
    const float XPUsed = CalculateXPForNextLevel(OldLevel);
    CurrentXP -= XPUsed;
    
    // 确保经验值不会变成负数（应该不会发生，但为了安全起见）
    CurrentXP = FMath::Max(0.0f, CurrentXP);

    // 授予技能和属性点
    SkillPoints += SkillPointsPerLevel;
    AttributePoints += AttributePointsPerLevel;

    // 应用升级奖励（属性增长等）
    ApplyLevelUpBonuses();

    // 广播事件
    OnLevelUpNative.Broadcast(OldLevel, Level);
    OnSkillPointChangedNative.Broadcast(SkillPoints);
    OnAttributePointChangedNative.Broadcast(AttributePoints);
    
    // 发送到 EventBus 供 UI 使用
    EmitLevelUpToEventBus(OldLevel, Level);
    EmitSkillPointsChangedToEventBus(SkillPoints);
    EmitAttributePointsChangedToEventBus(AttributePoints);
    
    // 同时更新 XP 显示（因为升级后 XP 可能变化）
    const float MaxXP = GetMaxXPForNextLevel();
    const float Percent = GetExpPercent();
    EmitXPChangedToEventBus(CurrentXP, MaxXP, Percent);

    UE_LOG(LogBMExperience, Log, TEXT("Level Up! %d -> %d. XP remaining: %f / %f. Skill Points: %d, Attribute Points: %d"),
        OldLevel, Level, CurrentXP, GetMaxXPForNextLevel(), SkillPoints, AttributePoints);

    return true;
}

void UBMExperienceComponent::ApplyLevelUpBonuses()
{
    // 根据 PlayerGrowthData 表中的数据应用属性增长
    if (!GetOwner())
    {
        UE_LOG(LogBMExperience, Warning, TEXT("ApplyLevelUpBonuses: Owner is null"));
        return;
    }

    UBMStatsComponent* Stats = GetOwner()->GetComponentByClass<UBMStatsComponent>();
    if (!Stats)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("ApplyLevelUpBonuses: Stats component not found"));
        return;
    }

    // 获取 DataSubsystem
    UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    if (!GameInstance)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("ApplyLevelUpBonuses: GameInstance not found"));
        return;
    }

    UBMDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UBMDataSubsystem>();
    if (!DataSubsystem)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("ApplyLevelUpBonuses: DataSubsystem not found"));
        return;
    }

    // 获取当前等级的成长数据
    const FBMPlayerGrowthData* PlayerGrowthData = DataSubsystem->GetPlayerGrowthData(Level);
    if (!PlayerGrowthData)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("ApplyLevelUpBonuses: PlayerGrowthData not found for level %d"), Level);
        return;
    }

    // 获取可修改的 StatBlock
    FBMStatBlock& StatBlock = Stats->GetStatBlockMutable();

    const float OldMaxHP = StatBlock.MaxHP;
    const float OldAttack = StatBlock.Attack;
    const float OldDefense = StatBlock.Defense;
    const float OldMaxStamina = StatBlock.MaxStamina;

    // 保存当前 HP 和 Stamina 的百分比，以便在更新最大值后保持比例
    const float HPRatio = StatBlock.MaxHP > 0.0f ? (StatBlock.HP / StatBlock.MaxHP) : 1.0f;
    const float StaminaRatio = StatBlock.MaxStamina > 0.0f ? (StatBlock.Stamina / StatBlock.MaxStamina) : 1.0f;

    // 应用成长数据表中的属性值
    StatBlock.MaxHP = PlayerGrowthData->MaxHP;
    StatBlock.Attack = PlayerGrowthData->AttackPower;
    StatBlock.Defense = PlayerGrowthData->Defense;
    StatBlock.MaxStamina = PlayerGrowthData->Stamina;

    const float OldHP = StatBlock.HP;
    const float OldStamina = StatBlock.Stamina;

    // 恢复 HP 和 Stamina，保持之前的百分比（但确保不超过最大值）
    StatBlock.HP = FMath::Clamp(StatBlock.MaxHP * HPRatio, 0.0f, StatBlock.MaxHP);
    StatBlock.Stamina = FMath::Clamp(StatBlock.MaxStamina * StaminaRatio, 0.0f, StatBlock.MaxStamina);

    UE_LOG(LogBMExperience, Log, TEXT("ApplyLevelUpBonuses: Applied growth data for level %d - MaxHP: %.1f, Attack: %.1f, Defense: %.1f, MaxStamina: %.1f"),
        Level, StatBlock.MaxHP, StatBlock.Attack, StatBlock.Defense, StatBlock.MaxStamina);
    
    // ===== 计算增益值 =====
    const float MaxHPGain = StatBlock.MaxHP - OldMaxHP;
    const float AttackGain = StatBlock.Attack - OldAttack;
    const float DefenseGain = StatBlock.Defense - OldDefense;
    const float MaxStaminaGain = StatBlock.MaxStamina - OldMaxStamina;
    const float HPRestored = StatBlock.HP - OldHP;
    const float StaminaRestored = StatBlock.Stamina - OldStamina;
    
    UE_LOG(LogBMExperience, Display, TEXT("========================================"));
    UE_LOG(LogBMExperience, Display, TEXT("Level Up Bonuses Applied for Level %d"), Level);
    UE_LOG(LogBMExperience, Display, TEXT("========================================"));
    
    // 生命值
    UE_LOG(LogBMExperience, Display, TEXT("MaxHP:      %.1f -> %.1f (+%.1f, %.1f%%)"), 
        OldMaxHP, StatBlock.MaxHP, MaxHPGain, 
        OldMaxHP > 0.0f ? (MaxHPGain / OldMaxHP * 100.0f) : 0.0f);
    UE_LOG(LogBMExperience, Display, TEXT("  Current HP: %.1f -> %.1f (+%.1f)"), 
        OldHP, StatBlock.HP, HPRestored);
    
    // 攻击力
    UE_LOG(LogBMExperience, Display, TEXT("Attack:     %.1f -> %.1f (+%.1f, %.1f%%)"), 
        OldAttack, StatBlock.Attack, AttackGain,
        OldAttack > 0.0f ? (AttackGain / OldAttack * 100.0f) : 0.0f);
    
    // 防御力
    UE_LOG(LogBMExperience, Display, TEXT("Defense:    %.1f -> %.1f (+%.1f, %.1f%%)"), 
        OldDefense, StatBlock.Defense, DefenseGain,
        OldDefense > 0.0f ? (DefenseGain / OldDefense * 100.0f) : 0.0f);
    
    // 耐力值
    UE_LOG(LogBMExperience, Display, TEXT("MaxStamina: %.1f -> %.1f (+%.1f, %.1f%%)"), 
        OldMaxStamina, StatBlock.MaxStamina, MaxStaminaGain,
        OldMaxStamina > 0.0f ? (MaxStaminaGain / OldMaxStamina * 100.0f) : 0.0f);
    UE_LOG(LogBMExperience, Display, TEXT("  Current Stamina: %.1f -> %.1f (+%.1f)"), 
        OldStamina, StatBlock.Stamina, StaminaRestored);
    
    UE_LOG(LogBMExperience, Display, TEXT("========================================"));
    
    // 额外奖励提示
    UE_LOG(LogBMExperience, Display, TEXT("Bonus Rewards: +%d Skill Point(s), +%d Attribute Point(s)"),
        SkillPointsPerLevel, AttributePointsPerLevel);
    UE_LOG(LogBMExperience, Display, TEXT("========================================"));

}

float UBMExperienceComponent::GetExpPercent() const
{
    const float MaxXP = GetMaxXPForNextLevel();
    if (MaxXP <= 0.0f)
    {
        return 1.0f; // At max level or invalid
    }
    return FMath::Clamp(CurrentXP / MaxXP, 0.0f, 1.0f);
}

bool UBMExperienceComponent::SpendSkillPoint()
{
    if (SkillPoints <= 0)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("SpendSkillPoint: No skill points available"));
        return false;
    }

    SkillPoints--;
    OnSkillPointChangedNative.Broadcast(SkillPoints);
    EmitSkillPointsChangedToEventBus(SkillPoints);
    UE_LOG(LogBMExperience, Log, TEXT("SpendSkillPoint: Spent 1 skill point. Remaining: %d"), SkillPoints);
    return true;
}

bool UBMExperienceComponent::SpendAttributePoint()
{
    if (AttributePoints <= 0)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("SpendAttributePoint: No attribute points available"));
        return false;
    }

    AttributePoints--;
    OnAttributePointChangedNative.Broadcast(AttributePoints);
    EmitAttributePointsChangedToEventBus(AttributePoints);
    UE_LOG(LogBMExperience, Log, TEXT("SpendAttributePoint: Spent 1 attribute point. Remaining: %d"), AttributePoints);
    return true;
}

void UBMExperienceComponent::SetLevel(int32 NewLevel, bool bApplyGrowth)
{
    if (NewLevel < 1)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("SetLevel: Invalid level %d, clamping to 1"), NewLevel);
        NewLevel = 1;
    }

    if (MaxLevel > 0 && NewLevel > MaxLevel)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("SetLevel: Level %d exceeds max level %d, clamping"), NewLevel, MaxLevel);
        NewLevel = MaxLevel;
    }

    const int32 OldLevel = Level;
    Level = NewLevel;

    // 如果应用增长，模拟升级
    if (bApplyGrowth && OldLevel < NewLevel)
    {
        // 暂时重置点数
        const int32 OldSkillPoints = SkillPoints;
        const int32 OldAttributePoints = AttributePoints;
        
        SkillPoints = 0;
        AttributePoints = 0;

        // 模拟升级以获取正确的点数总数
        for (int32 L = OldLevel; L < NewLevel; L++)
        {
            SkillPoints += SkillPointsPerLevel;
            AttributePoints += AttributePointsPerLevel;
            ApplyLevelUpBonuses();
        }

        // 恢复原始点数（从存档数据）
        SkillPoints = FMath::Max(SkillPoints, OldSkillPoints);
        AttributePoints = FMath::Max(AttributePoints, OldAttributePoints);
    }

    // 广播事件
    if (OldLevel != NewLevel)
    {
        OnLevelUpNative.Broadcast(OldLevel, NewLevel);
        EmitLevelUpToEventBus(OldLevel, NewLevel);
    }
    OnSkillPointChangedNative.Broadcast(SkillPoints);
    OnAttributePointChangedNative.Broadcast(AttributePoints);
    EmitSkillPointsChangedToEventBus(SkillPoints);
    EmitAttributePointsChangedToEventBus(AttributePoints);
    
    // 更新 XP 显示
    const float MaxXP = GetMaxXPForNextLevel();
    const float Percent = GetExpPercent();
    EmitXPChangedToEventBus(CurrentXP, MaxXP, Percent);

    UE_LOG(LogBMExperience, Log, TEXT("SetLevel: Set to level %d"), NewLevel);
}

void UBMExperienceComponent::SetCurrentXP(float NewXP, bool bCheckLevelUp)
{
    if (NewXP < 0.0f)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("SetCurrentXP: Negative XP %f, clamping to 0"), NewXP);
        NewXP = 0.0f;
    }

    CurrentXP = NewXP;

    // 检查是否需要升级
    if (bCheckLevelUp)
    {
        int32 LevelsGained = 0;
        while (CheckLevelUp() > 0)
        {
            LevelsGained++;
        }
        if (LevelsGained > 0)
        {
            UE_LOG(LogBMExperience, Log, TEXT("SetCurrentXP: Gained %d levels after setting XP"), LevelsGained);
        }
    }

    // 广播经验值变化
    const float MaxXP = GetMaxXPForNextLevel();
    const float Percent = GetExpPercent();
    OnXPChangedNative.Broadcast(CurrentXP, MaxXP, Percent);
    EmitXPChangedToEventBus(CurrentXP, MaxXP, Percent);
}

void UBMExperienceComponent::SetSkillPoints(int32 NewSkillPoints)
{
    if (NewSkillPoints < 0)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("SetSkillPoints: Negative value %d, clamping to 0"), NewSkillPoints);
        NewSkillPoints = 0;
    }

    SkillPoints = NewSkillPoints;
    OnSkillPointChangedNative.Broadcast(SkillPoints);
    EmitSkillPointsChangedToEventBus(SkillPoints);
}

void UBMExperienceComponent::SetAttributePoints(int32 NewAttributePoints)
{
    if (NewAttributePoints < 0)
    {
        UE_LOG(LogBMExperience, Warning, TEXT("SetAttributePoints: Negative value %d, clamping to 0"), NewAttributePoints);
        NewAttributePoints = 0;
    }

    AttributePoints = NewAttributePoints;
    OnAttributePointChangedNative.Broadcast(AttributePoints);
    EmitAttributePointsChangedToEventBus(AttributePoints);
}

float UBMExperienceComponent::CalculateXPForLevel(int32 TargetLevel) const
{
    if (TargetLevel <= 1)
    {
        return 0.0f;
    }

    // 指数增长公式：BaseXP * (Multiplier ^ (Level - 2))
    // 2级需要 BaseXP
    // 3级需要 BaseXP * Multiplier
    // 4级需要 BaseXP * Multiplier^2
    // 等等
    
    float TotalXP = 0.0f;
    for (int32 L = 2; L <= TargetLevel; L++)
    {
        TotalXP += CalculateXPForNextLevel(L - 1);
    }

    return TotalXP;
}

float UBMExperienceComponent::CalculateXPForNextLevel(int32 FromLevel) const
{
    if (FromLevel < 1)
    {
        return BaseXPRequired;
    }

    // 优先从 DataTable 读取
    UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    if (GameInstance)
    {
        UBMDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UBMDataSubsystem>();
        if (DataSubsystem)
        {
            const FBMPlayerGrowthData* GrowthData = DataSubsystem->GetPlayerGrowthData(FromLevel);
            if (GrowthData)
            {
                UE_LOG(LogBMExperience, Verbose, TEXT("CalculateXPForNextLevel: Level %d needs %f XP (from DataTable)"), FromLevel, GrowthData->ExpToNext);
                return GrowthData->ExpToNext;
            }
        }
    }

    // 如果 DataTable 中没有数据，回退到公式计算
    // 公式：BaseXP * (Multiplier ^ (FromLevel - 1))
    const float XPForThisLevel = BaseXPRequired * FMath::Pow(XPGrowthMultiplier, static_cast<float>(FromLevel - 1));
    UE_LOG(LogBMExperience, Warning, TEXT("CalculateXPForNextLevel: Level %d data not found in DataTable, using formula result: %f"), FromLevel, XPForThisLevel);
    return XPForThisLevel;
}

// === EventBus Integration ===

UBMEventBusSubsystem* UBMExperienceComponent::GetEventBusSubsystem() const
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UBMEventBusSubsystem>();
        }
    }
    return nullptr;
}

void UBMExperienceComponent::EmitLevelUpToEventBus(int32 OldLevel, int32 NewLevel)
{
    if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
    {
        EventBus->EmitPlayerLevelUp(OldLevel, NewLevel);
    }
}

void UBMExperienceComponent::EmitXPChangedToEventBus(float InCurrentXP, float InMaxXP, float InPercent)
{
    if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
    {
        EventBus->EmitPlayerXPChanged(InCurrentXP, InMaxXP, InPercent);
    }
}

void UBMExperienceComponent::EmitSkillPointsChangedToEventBus(int32 NewSkillPoints)
{
    if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
    {
        EventBus->EmitPlayerSkillPointsChanged(NewSkillPoints);
    }
}

void UBMExperienceComponent::EmitAttributePointsChangedToEventBus(int32 NewAttributePoints)
{
    if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
    {
        EventBus->EmitPlayerAttributePointsChanged(NewAttributePoints);
    }
}
