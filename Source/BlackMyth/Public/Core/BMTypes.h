// BMTypes.h
#pragma once

#include "CoreMinimal.h"
#include "BMTypes.generated.h"

// 预声明，供下面 USTRUCT 使用
class AActor;
class UObject;

/**
 * 角色阵营（友方/敌方/中立）
 */
UENUM(BlueprintType)
enum class EBMTeam : uint8
{
    Player      UMETA(DisplayName="Player"),
    Enemy       UMETA(DisplayName="Enemy"),
    Neutral     UMETA(DisplayName="Neutral")
};

/**
 * 角色大类（用于通用判断）
 */
UENUM(BlueprintType)
enum class EBMCharacterType : uint8
{
    Player      UMETA(DisplayName="Player"),
    Enemy       UMETA(DisplayName="Enemy"),
    Boss        UMETA(DisplayName="Boss"),
};

/**
 * 敌人具体类型（数据驱动时可用来区分配置）
 * 这里先列出几个示例，后续可以再扩展
 */
UENUM(BlueprintType)
enum class EBMEnemyType : uint8
{
    None        UMETA(DisplayName="None"),
    Goblin      UMETA(DisplayName="Goblin"),
    Wolf        UMETA(DisplayName="Wolf"),
    Monk        UMETA(DisplayName="Monk"),
    Boss        UMETA(DisplayName="Boss"),
};

/**
 * 技能槽位（用于 UI 显示技能冷却、快捷键映射）
 */
UENUM(BlueprintType)
enum class EBMSkillSlot : uint8
{
    Skill1      UMETA(DisplayName="Skill 1"),
    Skill2      UMETA(DisplayName="Skill 2"),
    Skill3      UMETA(DisplayName="Skill 3"),
    Ultimate    UMETA(DisplayName="Ultimate"),
};

/**
 * 伤害类型（后面可以做克制/抗性）
 */
UENUM(BlueprintType)
enum class EBMDamageType : uint8
{
    Melee       UMETA(DisplayName="Melee"),
    Fire        UMETA(DisplayName="Fire"),
    Poison      UMETA(DisplayName="Poison"),
    TrueDamage  UMETA(DisplayName="True Damage")
};

/**
 * 受击反馈类型（用于选择不同受击动画）
 */
UENUM(BlueprintType)
enum class EBMHitReaction : uint8
{
    None        UMETA(DisplayName="None"),
    Light       UMETA(DisplayName="Light Hit"),
    Heavy       UMETA(DisplayName="Heavy Hit"),
    KnockDown   UMETA(DisplayName="Knock Down"),
    Airborne    UMETA(DisplayName="Airborne"),
    Dead        UMETA(DisplayName="Dead")
};

/**
 * 角色逻辑状态 ID（FSM 使用的“状态枚举”）
 * 真正逻辑由具体 State 类实现，这里只是做一个统一 ID
 */
UENUM(BlueprintType)
enum class EBMCharacterStateId : uint8
{
    None        UMETA(DisplayName="None"),
    Idle        UMETA(DisplayName="Idle"),
    Move        UMETA(DisplayName="Move"),
    Jump        UMETA(DisplayName="Jump"),
    Roll        UMETA(DisplayName="Roll"),
    Attack      UMETA(DisplayName="Attack"),
    SkillCast   UMETA(DisplayName="Skill Cast"),
    Hit         UMETA(DisplayName="Hit"),
    Death       UMETA(DisplayName="Death")
};

/**
 * HitBox 类型（便于后续区分不同判定框行为）
 */
UENUM(BlueprintType)
enum class EBMHitBoxType : uint8
{
    WeaponSwing UMETA(DisplayName="Weapon Swing"),
    SkillArea   UMETA(DisplayName="Skill Area"),
    Projectile  UMETA(DisplayName="Projectile"),
};

/**
 * 游戏全局事件类型（EventBus 使用）
 * UI / Boss / Player / 系统之间通过这个来通信
 */
UENUM(BlueprintType)
enum class EBMEventType : uint8
{
    None                UMETA(DisplayName="None"),

    // 角色 & 战斗
    PlayerHPChanged     UMETA(DisplayName="Player HP Changed"),
    EnemyHPChanged      UMETA(DisplayName="Enemy HP Changed"),
    BossHPChanged       UMETA(DisplayName="Boss HP Changed"),
    PlayerDied          UMETA(DisplayName="Player Died"),
    BossDied            UMETA(DisplayName="Boss Died"),

    // Boss 阶段 & 状态
    BossPhaseChanged    UMETA(DisplayName="Boss Phase Changed"),

    // 技能 & 冷却
    SkillCooldownUpdated UMETA(DisplayName="Skill Cooldown Updated"),

    // 游戏流程
    GamePaused          UMETA(DisplayName="Game Paused"),
    GameResumed         UMETA(DisplayName="Game Resumed"),
};

/**
 * 属性块（Stats）—— 可给 Player / Enemy / Boss 公用
 * 和 UBMStatsComponent 中的属性一一对应
 */
USTRUCT(BlueprintType)
struct FBMStatBlock
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float MaxHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float HP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float MaxMP = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float MP = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float MaxStamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float Stamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float Attack = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float Defense = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float MoveSpeed = 600.f;
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    float DamageValue = 0.f;

    // 原始伤害值（可选，方便调试/显示）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    float RawDamageValue = 0.f;

    // 伤害类型（物理/火焰等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    EBMDamageType DamageType = EBMDamageType::Melee;

    // 是否暴击
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    bool bCritical = false;

    // 推荐受击反馈
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    EBMHitReaction HitReaction = EBMHitReaction::Light;

    // 击退力度（世界空间向量）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    FVector Knockback = FVector::ZeroVector;

    // 命中点 & 法线（方便做特效、血花方向等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    FVector HitNormal = FVector::ZeroVector;

    FBMDamageInfo() = default;
};

/**
 * 简单技能 ID（供数据驱动使用）
 * 你可以在配置表 / json 中用 FName 标识一个技能
 */
USTRUCT(BlueprintType)
struct FBMSkillId
{
    GENERATED_BODY()

    // 技能名，如 "CloudStep" / "EarthSmash"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
    FName Name = NAME_None;

    // 对应技能槽位（Skill1/Skill2/...）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skill")
    EBMSkillSlot Slot = EBMSkillSlot::Skill1;

    FBMSkillId() = default;
    FBMSkillId(FName InName, EBMSkillSlot InSlot)
        : Name(InName), Slot(InSlot)
    {}
};

/**
 * EventBus 用的事件数据结构
 * 统一所有系统之间的通信载体
 */
USTRUCT(BlueprintType)
struct FBMEventData
{
    GENERATED_BODY()

    // 事件类型（枚举）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
    EBMEventType Type = EBMEventType::None;

    // 可选：事件名，用于更细粒度区分（如 "PlayerHPChanged_Wukong"）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
    FName Name = NAME_None;

    // 谁发出的事件
    UPROPERTY(BlueprintReadWrite, Category="Event")
    TObjectPtr<UObject> Sender = nullptr;

    // 目标对象（比如 UI 想知道是哪个 Boss 的血条）
    UPROPERTY(BlueprintReadWrite, Category="Event")
    TObjectPtr<UObject> Target = nullptr;

    // 通用浮点参数（比如当前 HP、CD 等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Payload")
    float FloatValue = 0.f;

    // 通用整型参数（比如阶段 index、次数等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Payload")
    int32 IntValue = 0;

    // 通用向量参数（位置、方向等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Payload")
    FVector VectorValue = FVector::ZeroVector;

    // 额外的名字参数（可用于传技能 ID、Buff 名等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Payload")
    FName ExtraName = NAME_None;

    // 通用布尔参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Payload")
    bool bFlag = false;

    FBMEventData() = default;

    explicit FBMEventData(EBMEventType InType)
        : Type(InType)
    {}
};

/**
 * 一些常用事件名常量（配合 EBMEventType 使用，不想写死字符串时可以用）
 */
namespace BMEventNames
{
    // 血量相关
    static const FName PlayerHPChanged   = TEXT("PlayerHPChanged");
    static const FName EnemyHPChanged    = TEXT("EnemyHPChanged");
    static const FName BossHPChanged     = TEXT("BossHPChanged");

    // Boss 阶段
    static const FName BossPhaseChanged  = TEXT("BossPhaseChanged");

    // 技能冷却
    static const FName SkillCooldownUpdated = TEXT("SkillCooldownUpdated");

    // 游戏流程
    static const FName GamePaused        = TEXT("GamePaused");
    static const FName GameResumed       = TEXT("GameResumed");
}

/**
 * 一个简单的 AI 感知结果，用于敌人 AI / Boss AI
 */
USTRUCT(BlueprintType)
struct FBMPerceptionInfo
{
    GENERATED_BODY()

    // 是否已经看到玩家/锁定玩家
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perception")
    bool bCanSeePlayer = false;

    // 玩家是否在攻击范围内
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perception")
    bool bInAttackRange = false;

    // 玩家当前位置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perception")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
};
