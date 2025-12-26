#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMStatsComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBMStats, Log, All);


DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnDeathNative, AActor* /*Killer*/);

/**
 * 临时增益效果类型
 */
UENUM(BlueprintType)
enum class EBMBuffType : uint8
{
    None                UMETA(DisplayName = "None"),
    AttackBoost         UMETA(DisplayName = "Attack Boost"),        // 攻击力百分比加成
    Invulnerability     UMETA(DisplayName = "Invulnerability"),     // 无敌
    StaminaRegenBoost   UMETA(DisplayName = "Stamina Regen Boost"), // 耐力恢复加速
    HealthRegenOverTime UMETA(DisplayName = "Health Regen Over Time"), // 持续回血
    MaxHpBoost          UMETA(DisplayName = "Max HP Boost")         // 最大生命值临时提升
};

/**
 * 单个增益效果实例
 */
USTRUCT(BlueprintType)
struct FBMBuffInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    EBMBuffType Type = EBMBuffType::None;

    // 效果数值
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    float Value = 0.f;

    // 剩余持续时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    float RemainingTime = 0.f;

    // 总持续时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    float TotalDuration = 0.f;

    FBMBuffInstance() = default;
    FBMBuffInstance(EBMBuffType InType, float InValue, float InDuration)
        : Type(InType), Value(InValue), RemainingTime(InDuration), TotalDuration(InDuration)
    {
    }

    bool IsExpired() const { return RemainingTime <= 0.f; }
};

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMStatsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMStatsComponent();

    virtual void BeginPlay() override;

    float ApplyDamage(FBMDamageInfo& InOutInfo);

    bool IsDead() const { return Stats.HP <= 0.f; }

    bool TryConsumeStamina(float Amount);

    bool TryConsumeMP(float Amount);

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void AddGameplayTag(FName Tag);
    void RemoveGameplayTag(FName Tag);
    bool HasGameplayTag(FName Tag) const;

    const FBMStatBlock& GetStatBlock() const { return Stats; }
    FBMStatBlock& GetStatBlockMutable() { return Stats; }
    void ReviveToFull(float NewMaxHP);
    void InitializeFromBlock(const FBMStatBlock& In);

    // Revive the owner by restoring HP to MaxHP and clearing death state
    void Revive();

    // ==================== 道具效果接口 ====================
    
    /**
     * 立即恢复HP（百分比）
     * @param Percent HP百分比 (0-100)
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Item")
    void HealByPercent(float Percent);

    /**
     * 立即恢复HP（固定值）
     * @param Amount HP固定值
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Item")
    void HealByAmount(float Amount);

    /**
     * 添加临时增益效果
     * @param Type 增益类型
     * @param Value 效果数值
     * @param Duration 持续时间（秒）
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    void AddBuff(EBMBuffType Type, float Value, float Duration);

    /**
     * 移除指定类型的增益效果
     * @param Type 增益类型
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    void RemoveBuff(EBMBuffType Type);

    /**
     * 检查是否有指定类型的增益效果
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    bool HasBuff(EBMBuffType Type) const;

    /**
     * 检查角色是否处于无敌状态
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    bool IsInvulnerable() const;

    /**
     * 获取当前攻击力加成倍率（1.0 = 无加成）
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    float GetAttackMultiplier() const;

    /**
     * 获取当前耐力恢复加成倍率（1.0 = 无加成）
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    float GetStaminaRegenMultiplier() const;

    /**
     * 获取当前最大生命值（包含临时加成）
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    float GetEffectiveMaxHp() const;

    /**
     * 获取所有活跃的增益效果
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    TArray<FBMBuffInstance> GetActiveBuffs() const { return ActiveBuffs; }

public:
    FBMOnDeathNative OnDeathNative;

private:
    UPROPERTY(EditAnywhere, Category = "BM|Stats")
    FBMStatBlock Stats;

    // 耐力每秒恢复速度（基础值）
    UPROPERTY(EditAnywhere, Category = "BM|Stats", meta = (ClampMin = "0.0"))
    float StaminaRegenPerSec = 10.f;

    // 活跃的临时增益效果列表
    UPROPERTY(Transient)
    TArray<FBMBuffInstance> ActiveBuffs;

    bool bDeathBroadcasted = false;
    TSet<FName> Tags;

    // 内部辅助：更新增益效果计时器
    void TickBuffs(float DeltaTime);

    // 内部辅助：处理持续回血效果
    void TickHealthRegen(float DeltaTime);
};
