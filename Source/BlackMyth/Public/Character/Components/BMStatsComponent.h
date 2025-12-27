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
 *
 * 存储临时增益效果的类型、数值和剩余时间
 */
USTRUCT(BlueprintType)
struct FBMBuffInstance
{
    GENERATED_BODY()

    /** 增益效果类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    EBMBuffType Type = EBMBuffType::None;

    /** 效果数值 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    float Value = 0.f;

    /** 剩余持续时间（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    float RemainingTime = 0.f;

    /** 总持续时间（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    float TotalDuration = 0.f;

    FBMBuffInstance() = default;

    /**
     * 参数化构造函数
     *
     * @param InType 增益类型
     * @param InValue 效果数值
     * @param InDuration 持续时间
     */
    FBMBuffInstance(EBMBuffType InType, float InValue, float InDuration)
        : Type(InType), Value(InValue), RemainingTime(InDuration), TotalDuration(InDuration)
    {
    }

    /** 判断增益是否已过期 */
    bool IsExpired() const { return RemainingTime <= 0.f; }
};

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMStatsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMStatsComponent();

    /**
     * 组件开始运行
     *
     * 初始化并通知 UI 系统当前血量和耐力值
     */
    virtual void BeginPlay() override;

    /**
     * 应用伤害
     *
     * 计算防御减伤、真实伤害等，更新 HP 并触发死亡事件
     * 无敌状态下伤害为 0
     *
     * @param InOutInfo 伤害信息
     * @return 实际应用的伤害值
     */
    float ApplyDamage(FBMDamageInfo& InOutInfo);

    /**
     * 判断角色是否已死亡
     *
     * @return HP <= 0 时返回 true
     */
    bool IsDead() const { return Stats.HP <= 0.f; }

    /**
     * 尝试消耗耐力
     *
     * @param Amount 要消耗的耐力值
     * @return 耐力足够并成功消耗返回 true，否则返回 false
     */
    bool TryConsumeStamina(float Amount);

    /**
     * 尝试消耗 MP
     *
     * @param Amount 要消耗的 MP 值
     * @return MP 足够并成功消耗返回 true，否则返回 false
     */
    bool TryConsumeMP(float Amount);

    /**
     * 每帧更新
     *
     * 更新增益效果计时器、处理持续回血、恢复耐力并同步 UI
     *
     * @param DeltaTime 帧时间间隔
     * @param TickType Tick 类型
     * @param ThisTickFunction Tick 函数指针
     */
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /**
     * 添加游戏标签
     *
     * @param Tag 要添加的标签名称
     */
    void AddGameplayTag(FName Tag);

    /**
     * 移除游戏标签
     *
     * @param Tag 要移除的标签名称
     */
    void RemoveGameplayTag(FName Tag);

    /**
     * 检查是否拥有指定标签
     *
     * @param Tag 要检查的标签名称
     * @return 拥有该标签返回 true
     */
    bool HasGameplayTag(FName Tag) const;

    /** 获取属性数据块（只读） */
    const FBMStatBlock& GetStatBlock() const { return Stats; }

    /** 获取属性数据块（可修改） */
    FBMStatBlock& GetStatBlockMutable() { return Stats; }

    /**
     * 复活至满血
     *
     * @param NewMaxHP 新的最大生命值
     */
    void ReviveToFull(float NewMaxHP);

    /**
     * 从属性块初始化
     *
     * @param In 源属性数据块
     */
    void InitializeFromBlock(const FBMStatBlock& In);

    /**
     * 复活角色
     *
     * 恢复 HP 至 MaxHP 并清除死亡状态，通知 UI 更新血量
     */
    void Revive();
    
    /**
     * 立即恢复 HP（百分比）
     *
     * 根据最大生命值百分比恢复 HP，死亡状态下无效
     *
     * @param Percent HP 百分比 (0-100)
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Item")
    void HealByPercent(float Percent);

    /**
     * 立即恢复 HP（固定值）
     *
     * 恢复指定数值的 HP，不超过有效最大生命值，死亡状态下无效
     *
     * @param Amount HP 固定值
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Item")
    void HealByAmount(float Amount);

    /**
     * 添加临时增益效果
     *
     * 移除同类型旧效果后添加新效果
     *
     * @param Type 增益类型
     * @param Value 效果数值
     * @param Duration 持续时间
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    void AddBuff(EBMBuffType Type, float Value, float Duration);

    /**
     * 移除指定类型的增益效果
     *
     * @param Type 增益类型
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    void RemoveBuff(EBMBuffType Type);

    /**
     * 检查是否有指定类型的增益效果
     *
     * @param Type 增益类型
     * @return 拥有该类型未过期的增益效果返回 true
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    bool HasBuff(EBMBuffType Type) const;

    /**
     * 检查角色是否处于无敌状态
     *
     * @return 拥有无敌增益效果返回 true
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    bool IsInvulnerable() const;

    /**
     * 获取当前攻击力加成倍率
     *
     * @return 攻击力倍率
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    float GetAttackMultiplier() const;

    /**
     * 获取当前耐力恢复加成倍率
     *
     * @return 耐力恢复倍率
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    float GetStaminaRegenMultiplier() const;

    /**
     * 获取当前有效最大生命值
     *
     * @return 包含临时加成的最大生命值
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    float GetEffectiveMaxHp() const;

    /**
     * 获取所有活跃的增益效果
     *
     * @return 当前所有增益效果列表
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Stats|Buff")
    TArray<FBMBuffInstance> GetActiveBuffs() const { return ActiveBuffs; }

public:
    /** 死亡事件 */
    FBMOnDeathNative OnDeathNative;

private:
    /** 角色属性数据块 */
    UPROPERTY(EditAnywhere, Category = "BM|Stats")
    FBMStatBlock Stats;

    /** 耐力每秒恢复速度 */
    UPROPERTY(EditAnywhere, Category = "BM|Stats", meta = (ClampMin = "0.0"))
    float StaminaRegenPerSec = 10.f;

    /** 活跃的临时增益效果列表 */
    UPROPERTY(Transient)
    TArray<FBMBuffInstance> ActiveBuffs;

    /** 死亡事件是否已广播 */
    bool bDeathBroadcasted = false;

    /** 游戏标签集合 */
    TSet<FName> Tags;

    /**
     * 更新增益效果计时器
     *
     * 减少剩余时间并移除过期效果，MaxHpBoost 过期时调整当前 HP
     *
     * @param DeltaTime 帧时间间隔
     */
    void TickBuffs(float DeltaTime);

    /**
     * 处理持续回血效果
     *
     * 计算并应用持续回血增益的每帧恢复量
     *
     * @param DeltaTime 帧时间间隔
     */
    void TickHealthRegen(float DeltaTime);
};
