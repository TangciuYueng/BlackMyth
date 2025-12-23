#include "Character/Components/BMSkillComponent.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMStatsComponent.h"
#include "Core/BMDataSubsystem.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogBMSkill);

UBMSkillComponent::UBMSkillComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
    SkillID = NAME_None;
    SkillData = nullptr;
    LastUsedTime = 0.0f;
    bIsSkillActive = false;
}

void UBMSkillComponent::BeginPlay()
{
    Super::BeginPlay();

    // 如果 SkillID 已在编辑器中设置，自动初始化技能数据
    if (!SkillID.IsNone())
    {
        InitializeSkill(SkillID);
    }
}

bool UBMSkillComponent::InitializeSkill(FName InSkillID)
{
    if (InSkillID.IsNone())
    {
        UE_LOG(LogBMSkill, Warning, TEXT("InitializeSkill: SkillID is None"));
        return false;
    }

    // 获取数据子系统
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogBMSkill, Error, TEXT("InitializeSkill: Failed to get World"));
        return false;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(LogBMSkill, Error, TEXT("InitializeSkill: Failed to get GameInstance"));
        return false;
    }

    UBMDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UBMDataSubsystem>();
    if (!DataSubsystem)
    {
        UE_LOG(LogBMSkill, Error, TEXT("InitializeSkill: Failed to get BMDataSubsystem"));
        return false;
    }

    // 从数据子系统加载技能数据
    SkillData = DataSubsystem->GetSkillData(InSkillID);
    if (!SkillData)
    {
        UE_LOG(LogBMSkill, Warning, TEXT("InitializeSkill: Failed to load skill data for SkillID: %s"), *InSkillID.ToString());
        return false;
    }

    SkillID = InSkillID;
    LastUsedTime = 0.0f;
    bIsSkillActive = false;

    UE_LOG(LogBMSkill, Log, TEXT("InitializeSkill: Successfully initialized skill %s"), *InSkillID.ToString());
    return true;
}

bool UBMSkillComponent::Execute(ABMPlayerCharacter* PlayerCharacter)
{
    if (!PlayerCharacter)
    {
        UE_LOG(LogBMSkill, Warning, TEXT("Execute: PlayerCharacter is null"));
        return false;
    }

    // 检查技能是否可用
    if (!IsAvailable())
    {
        UE_LOG(LogBMSkill, Verbose, TEXT("Execute: Skill %s is not available"), *SkillID.ToString());
        return false;
    }

    // 检查资源是否足够
    if (!CheckResourceAvailable(PlayerCharacter))
    {
        UE_LOG(LogBMSkill, Verbose, TEXT("Execute: Insufficient resources for skill %s"), *SkillID.ToString());
        return false;
    }

    // 消耗资源
    if (!ConsumeResource(PlayerCharacter))
    {
        UE_LOG(LogBMSkill, Warning, TEXT("Execute: Failed to consume resources for skill %s"), *SkillID.ToString());
        return false;
    }

    // 更新使用时间
    UWorld* World = GetWorld();
    if (World)
    {
        LastUsedTime = World->GetTimeSeconds();
    }

    // 设置激活状态
    bIsSkillActive = true;

    // 广播技能执行事件
    OnSkillExecuted.Broadcast(SkillID, true);

    UE_LOG(LogBMSkill, Log, TEXT("Execute: Skill %s executed successfully"), *SkillID.ToString());
    return true;
}

float UBMSkillComponent::GetCost() const
{
    if (!SkillData)
    {
        return 0.0f;
    }
    return SkillData->Cost;
}

float UBMSkillComponent::GetCooldown() const
{
    if (!SkillData)
    {
        return 0.0f;
    }
    return SkillData->Cooldown;
}

bool UBMSkillComponent::IsAvailable() const
{
    // 检查技能配置是否有效
    if (!SkillData)
    {
        return false; // 技能数据未加载
    }

    // 检查是否处于冷却中
    UWorld* World = GetWorld();
    if (World)
    {
        const float CurrentTime = World->GetTimeSeconds();
        const float CooldownTime = GetCooldown();
        
        if (CooldownTime > 0.0f && (CurrentTime - LastUsedTime) < CooldownTime)
        {
            return false; // 仍在冷却中
        }
    }

    // 检查是否已经处于激活状态
    if (bIsSkillActive)
    {
        return false; // 技能正在执行中
    }

    // 注意：资源检查在 Execute 中进行，因为需要 PlayerCharacter 参数
    // 这里只检查冷却和激活状态
    return true;
}

float UBMSkillComponent::GetRemainingCooldown() const
{
    if (!SkillData)
    {
        return 0.0f;
    }

    const float CooldownTime = GetCooldown();
    if (CooldownTime <= 0.0f)
    {
        return 0.0f; // 无冷却时间
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }

    const float CurrentTime = World->GetTimeSeconds();
    const float ElapsedTime = CurrentTime - LastUsedTime;
    const float Remaining = CooldownTime - ElapsedTime;

    // 返回剩余时间，如果已过冷却则返回 0
    return FMath::Max(0.0f, Remaining);
}

bool UBMSkillComponent::CheckResourceAvailable(ABMPlayerCharacter* PlayerCharacter) const
{
    if (!PlayerCharacter)
    {
        return false;
    }

    UBMStatsComponent* Stats = PlayerCharacter->GetStats();
    if (!Stats)
    {
        UE_LOG(LogBMSkill, Warning, TEXT("CheckResourceAvailable: Stats component not found"));
        return false;
    }

    const float Cost = GetCost();
    if (Cost <= 0.0f)
    {
        return true; // 无消耗，直接返回可用
    }

    // 检查 Stamina 是否足够
    // 技能消耗 Stamina 而不是 MP
    const FBMStatBlock& StatBlock = Stats->GetStatBlock();
    return StatBlock.Stamina >= Cost;
}

bool UBMSkillComponent::ConsumeResource(ABMPlayerCharacter* PlayerCharacter) const
{
    if (!PlayerCharacter)
    {
        return false;
    }

    UBMStatsComponent* Stats = PlayerCharacter->GetStats();
    if (!Stats)
    {
        UE_LOG(LogBMSkill, Warning, TEXT("ConsumeResource: Stats component not found"));
        return false;
    }

    const float Cost = GetCost();
    if (Cost <= 0.0f)
    {
        return true; // 无消耗，直接返回成功
    }

    // 消耗 Stamina
    // 技能消耗 Stamina 而不是 MP
    return Stats->TryConsumeStamina(Cost);
}

