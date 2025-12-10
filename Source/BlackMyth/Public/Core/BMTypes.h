// BMTypes.h
#pragma once

#include "CoreMinimal.h"
#include "BMTypes.generated.h"

// 预声明，供下面 USTRUCT 使用
class AActor;
class UObject;
class UPrimitiveComponent;

/**
 * 输入设备类型（PC/手柄切换等）
 */
UENUM(BlueprintType)
enum class EInputDeviceType : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    KeyboardMouse UMETA(DisplayName = "Keyboard & Mouse"),
    Gamepad     UMETA(DisplayName = "Gamepad"),
    Touch       UMETA(DisplayName = "Touch")
};

/**
 * UI 通知类型（Toast/提示框等）
 */
UENUM(BlueprintType)
enum class ENotificationType : uint8
{
    Info        UMETA(DisplayName = "Info"),
    Success     UMETA(DisplayName = "Success"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error")
};

/**
 * 角色阵营（友方/敌方/中立）
 */
UENUM(BlueprintType)
enum class EBMTeam : uint8
{
    Player      UMETA(DisplayName = "Player"),
    Enemy       UMETA(DisplayName = "Enemy"),
    Neutral     UMETA(DisplayName = "Neutral")
};

/**
 * 角色大类（用于通用判断）
 */
UENUM(BlueprintType)
enum class EBMCharacterType : uint8
{
    Player      UMETA(DisplayName = "Player"),
    Enemy       UMETA(DisplayName = "Enemy"),
    Boss        UMETA(DisplayName = "Boss"),
};

/**
 * 敌人具体类型（数据驱动时可用来区分配置）
 */
UENUM(BlueprintType)
enum class EBMEnemyType : uint8
{
    None        UMETA(DisplayName = "None"),
    Goblin      UMETA(DisplayName = "Goblin"),
    Wolf        UMETA(DisplayName = "Wolf"),
    Monk        UMETA(DisplayName = "Monk"),
    Boss        UMETA(DisplayName = "Boss"),
};

/**
 * 技能槽位（用于 UI 显示技能冷却、快捷键映射）
 */
UENUM(BlueprintType)
enum class EBMSkillSlot : uint8
{
    Skill1      UMETA(DisplayName = "Skill 1"),
    Skill2      UMETA(DisplayName = "Skill 2"),
    Skill3      UMETA(DisplayName = "Skill 3"),
    Ultimate    UMETA(DisplayName = "Ultimate"),
};

/**
 * 伤害元素类型（克制/抗性）
 */
UENUM(BlueprintType)
enum class EBMElementType : uint8
{
    None        UMETA(DisplayName = "None"),
    Physical    UMETA(DisplayName = "Physical"),
    Fire        UMETA(DisplayName = "Fire"),
    Ice         UMETA(DisplayName = "Ice"),
    Lightning   UMETA(DisplayName = "Lightning"),
    Poison      UMETA(DisplayName = "Poison")
};

/**
 * 伤害“来源/表现”类型（你的 FBMDamageInfo 里用到，但原文件缺失）
 * - ElementType 解决“属性/克制”
 * - DamageType 解决“来源/机制”（近战/远程/法术/持续/真伤）
 */
UENUM(BlueprintType)
enum class EBMDamageType : uint8
{
    Melee       UMETA(DisplayName = "Melee"),
    Ranged      UMETA(DisplayName = "Ranged"),
    Magic       UMETA(DisplayName = "Magic"),
    DOT         UMETA(DisplayName = "Damage Over Time"),
    TrueDamage  UMETA(DisplayName = "True Damage")
};

/**
 * 受击反馈类型（用于选择不同受击动画）
 */
UENUM(BlueprintType)
enum class EBMHitReaction : uint8
{
    None        UMETA(DisplayName = "None"),
    Light       UMETA(DisplayName = "Light Hit"),
    Heavy       UMETA(DisplayName = "Heavy Hit"),
    KnockDown   UMETA(DisplayName = "Knock Down"),
    Airborne    UMETA(DisplayName = "Airborne"),
    Dead        UMETA(DisplayName = "Dead")
};

/**
 * 角色逻辑状态 ID（FSM 使用的“状态枚举”）
 */
UENUM(BlueprintType)
enum class EBMCharacterStateId : uint8
{
    None        UMETA(DisplayName = "None"),
    Idle        UMETA(DisplayName = "Idle"),
    Move        UMETA(DisplayName = "Move"),
    Jump        UMETA(DisplayName = "Jump"),
    Roll        UMETA(DisplayName = "Roll"),
    Attack      UMETA(DisplayName = "Attack"),
    SkillCast   UMETA(DisplayName = "Skill Cast"),
    Hit         UMETA(DisplayName = "Hit"),
    Death       UMETA(DisplayName = "Death")
};

/**
 * HitBox 类型（区分不同判定框行为）
 */
UENUM(BlueprintType)
enum class EBMHitBoxType : uint8
{
    Default     UMETA(DisplayName = "Default"),
    LightAttack UMETA(DisplayName = "Light Attack"),
    HeavyAttack UMETA(DisplayName = "Heavy Attack"),
    Skill       UMETA(DisplayName = "Skill")
};

/**
 * 游戏全局事件类型（EventBus 使用）
 */
UENUM(BlueprintType)
enum class EBMEventType : uint8
{
    None                UMETA(DisplayName = "None"),

    // 角色 & 战斗
    PlayerHPChanged     UMETA(DisplayName = "Player HP Changed"),
    EnemyHPChanged      UMETA(DisplayName = "Enemy HP Changed"),
    BossHPChanged       UMETA(DisplayName = "Boss HP Changed"),
    PlayerDied          UMETA(DisplayName = "Player Died"),
    BossDied            UMETA(DisplayName = "Boss Died"),

    // Boss 阶段 & 状态
    BossPhaseChanged    UMETA(DisplayName = "Boss Phase Changed"),

    // 技能 & 冷却
    SkillCooldownUpdated UMETA(DisplayName = "Skill Cooldown Updated"),

    // 游戏流程
    GamePaused          UMETA(DisplayName = "Game Paused"),
    GameResumed         UMETA(DisplayName = "Game Resumed"),
};

/**
 * 物品稀有度（可选：掉落/UI展示）
 */
UENUM(BlueprintType)
enum class EBMItemRarity : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Uncommon    UMETA(DisplayName = "Uncommon"),
    Rare        UMETA(DisplayName = "Rare"),
    Epic        UMETA(DisplayName = "Epic"),
    Legendary   UMETA(DisplayName = "Legendary")
};

/**
 * 物品大类（可选：背包分类/掉落表）
 */
UENUM(BlueprintType)
enum class EBMItemType : uint8
{
    None        UMETA(DisplayName = "None"),
    Material    UMETA(DisplayName = "Material"),
    Consumable  UMETA(DisplayName = "Consumable"),
    Equipment   UMETA(DisplayName = "Equipment"),
    Quest       UMETA(DisplayName = "Quest")
};

/**
 * 属性块（Stats）—— Player / Enemy / Boss 公用
 */
USTRUCT(BlueprintType)
struct FBMStatBlock
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float HP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxMP = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MP = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxStamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Attack = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Defense = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MoveSpeed = 600.f;
};

/**
 * 存档：背包物品条目（你类图里用到的 FMBInventoryItemSaveData）
 */
USTRUCT(BlueprintType)
struct FMBInventoryItemSaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FName ItemID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Quantity = 0;
};

/**
 * 存档：基础元信息（可用于“读取存档列表”UI展示）
 */
USTRUCT(BlueprintType)
struct FBMSaveMeta
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
    FName MapName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
    FRotator PlayerRotation = FRotator::ZeroRotator;
};

/**
 * 掉落表条目（敌人 LootTable 使用）
 */
USTRUCT(BlueprintType)
struct FBMLootItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    FName ItemID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    EBMItemType ItemType = EBMItemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    EBMItemRarity Rarity = EBMItemRarity::Common;

    // 掉落概率 [0,1]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Probability = 1.0f;

    // 掉落数量范围（含）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0"))
    int32 MinQuantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0"))
    int32 MaxQuantity = 1;

    // 可选：权重（你如果以后用“权重随机”而不是概率，也能直接用）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0.0"))
    float Weight = 0.0f;
};

/**
 * 伤害信息（HitBox 计算完毕之后，用这个结构传给目标）
 */
USTRUCT(BlueprintType)
struct FBMDamageInfo
{
    GENERATED_BODY()

    // 谁造成了伤害
    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<AActor> InstigatorActor = nullptr;

    // 谁被打到了
    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<AActor> TargetActor = nullptr;

    // 实际伤害值（已经考虑攻击/防御/倍率之后）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float DamageValue = 0.f;

    // 原始伤害值（可选，方便调试/显示）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float RawDamageValue = 0.f;

    // 伤害“来源/机制”
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EBMDamageType DamageType = EBMDamageType::Melee;

    // 元素属性（克制/抗性）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EBMElementType ElementType = EBMElementType::Physical;

    // 是否暴击
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bCritical = false;

    // 推荐受击反馈
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EBMHitReaction HitReaction = EBMHitReaction::Light;

    // 击退力度（世界空间向量）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector Knockback = FVector::ZeroVector;

    // 命中点 & 法线（方便做特效、血花方向等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FVector HitNormal = FVector::ZeroVector;

    // 可选：命中的组件（例如 HurtBox 判断头部/躯干倍率）
    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

    FBMDamageInfo() = default;
};

/**
 * 简单技能 ID（供数据驱动使用）
 */
USTRUCT(BlueprintType)
struct FBMSkillId
{
    GENERATED_BODY()

    // 技能名，如 "CloudStep" / "EarthSmash"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    FName Name = NAME_None;

    // 对应技能槽位（Skill1/Skill2/...）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    EBMSkillSlot Slot = EBMSkillSlot::Skill1;

    FBMSkillId() = default;
    FBMSkillId(FName InName, EBMSkillSlot InSlot)
        : Name(InName), Slot(InSlot)
    {
    }
};

/**
 * 技能冷却信息（UI/事件总线通用载体）
 */
USTRUCT(BlueprintType)
struct FBMSkillCooldownInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    EBMSkillSlot Slot = EBMSkillSlot::Skill1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float CooldownRemaining = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float CooldownTotal = 0.f;
};

/**
 * EventBus 用的事件数据结构
 */
USTRUCT(BlueprintType)
struct FBMEventData
{
    GENERATED_BODY()

    // 事件类型（枚举）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    EBMEventType Type = EBMEventType::None;

    // 可选：事件名（更细粒度区分）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FName Name = NAME_None;

    // 谁发出的事件
    UPROPERTY(BlueprintReadWrite, Category = "Event")
    TObjectPtr<UObject> Sender = nullptr;

    // 目标对象（比如 UI 想知道是哪个 Boss 的血条）
    UPROPERTY(BlueprintReadWrite, Category = "Event")
    TObjectPtr<UObject> Target = nullptr;

    // 通用浮点参数（比如当前 HP、CD 等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    float FloatValue = 0.f;

    // 通用整型参数（比如阶段 index、次数等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    int32 IntValue = 0;

    // 通用向量参数（位置、方向等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    FVector VectorValue = FVector::ZeroVector;

    // 额外的名字参数（可用于传技能 ID、Buff 名等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    FName ExtraName = NAME_None;

    // 通用布尔参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    bool bFlag = false;

    // 可选：常用载体（不用就忽略）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    FBMDamageInfo DamageInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Payload")
    FBMSkillCooldownInfo SkillCooldown;

    FBMEventData() = default;

    explicit FBMEventData(EBMEventType InType)
        : Type(InType)
    {
    }
};

/**
 * 常用事件名常量（配合 EBMEventType 使用）
 */
namespace BMEventNames
{
    // 血量相关
    static const FName PlayerHPChanged = TEXT("PlayerHPChanged");
    static const FName EnemyHPChanged = TEXT("EnemyHPChanged");
    static const FName BossHPChanged = TEXT("BossHPChanged");

    // Boss 阶段
    static const FName BossPhaseChanged = TEXT("BossPhaseChanged");

    // 技能冷却
    static const FName SkillCooldownUpdated = TEXT("SkillCooldownUpdated");

    // 游戏流程
    static const FName GamePaused = TEXT("GamePaused");
    static const FName GameResumed = TEXT("GameResumed");
}

/**
 * 一个简单的 AI 感知结果，用于敌人 AI / Boss AI
 */
USTRUCT(BlueprintType)
struct FBMPerceptionInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    bool bCanSeePlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    bool bInAttackRange = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
};

/**
 * 轻量辅助：把 StateId 映射成 FName（给你的 FSM 用）
 * 说明：FSM 内部如果用 FName 做 Key，这个函数能避免散落魔法字符串。
 */
namespace BMStateNames
{
    static const FName None = TEXT("None");
    static const FName Idle = TEXT("Idle");
    static const FName Move = TEXT("Move");
    static const FName Jump = TEXT("Jump");
    static const FName Roll = TEXT("Roll");
    static const FName Attack = TEXT("Attack");
    static const FName SkillCast = TEXT("SkillCast");
    static const FName Hit = TEXT("Hit");
    static const FName Death = TEXT("Death");

    FORCEINLINE FName ToName(EBMCharacterStateId Id)
    {
        switch (Id)
        {
            case EBMCharacterStateId::Idle:      return Idle;
            case EBMCharacterStateId::Move:      return Move;
            case EBMCharacterStateId::Jump:      return Jump;
            case EBMCharacterStateId::Roll:      return Roll;
            case EBMCharacterStateId::Attack:    return Attack;
            case EBMCharacterStateId::SkillCast: return SkillCast;
            case EBMCharacterStateId::Hit:       return Hit;
            case EBMCharacterStateId::Death:     return Death;
            default:                             return None;
        }
    }
}
