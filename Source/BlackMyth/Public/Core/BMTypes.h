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
 * 敌人攻击权重分类（用于 AI 攻击选择）
 */ 
UENUM(BlueprintType)
enum class EBMEnemyAttackWeight : uint8
{
    Light   UMETA(DisplayName = "Light"),
    Heavy   UMETA(DisplayName = "Heavy"),
    Skill   UMETA(DisplayName = "Skill")
};

/**
 * 玩家攻击请求类型
 */ 

UENUM(BlueprintType)
enum class EBMCombatAction : uint8
{
    None        UMETA(DisplayName = "None"),
    NormalAttack UMETA(DisplayName = "NormalAttack"), 
    Dodge       UMETA(DisplayName = "Dodge"),
    Skill1      UMETA(DisplayName = "Skill 1"),
    Skill2      UMETA(DisplayName = "Skill 2"),
    Skill3      UMETA(DisplayName = "Skill 3"),
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
    Dodge       UMETA(DisplayName = "Dodge"),
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

UENUM(BlueprintType)
enum class EBMHitDedupPolicy : uint8
{
    PerWindow,      // 默认：一次攻击窗口内，每个目标最多结算 MaxHitsPerTarget 次（通常=1）
    PerHitBox,      // 每个目标对每个 HitBoxName 分开计数
    Unlimited       // 不去重（不建议默认用）
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

USTRUCT(BlueprintType)
struct FBMHitBoxActivationParams
{
    GENERATED_BODY()

    // 这次窗口是谁触发的（可用于调试/回放/技能系统）
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Window")
    FName AttackId = NAME_None;

    // 这次窗口的额外倍率（用于同一套 HitBox 定义在不同招式里复用）
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Window", meta = (ClampMin = "0.0"))
    float DamageMultiplier = 1.0f;

    // 是否在窗口开始时清空命中记录
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Window")
    bool bResetHitRecords = true;

    // 去重策略
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Window")
    EBMHitDedupPolicy DedupPolicy = EBMHitDedupPolicy::PerWindow;

    // 每个目标允许命中次数（PerWindow 下通常=1；做多段斩可以调大）
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Window", meta = (ClampMin = "1"))
    int32 MaxHitsPerTarget = 1;

    // 可选：覆写受击类型（某些技能想强制 KnockDown 等）
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Window")
    bool bOverrideReaction = false;

    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Window")
    EBMHitReaction OverrideReaction = EBMHitReaction::None;
};

/**
 * 敌人攻击规格
 */ 
USTRUCT(BlueprintType)
struct FBMEnemyAttackSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack")
    TObjectPtr<UAnimSequence> Anim = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Attack")
    FName Id = NAME_None;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack", meta = (ClampMin = "0.01"))
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack", meta = (ClampMin = "0"))
    float MinRange = 0.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack", meta = (ClampMin = "0"))
    float MaxRange = 220.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack", meta = (ClampMin = "0.05"))
    float Cooldown = 1.2f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack", meta = (ClampMin = "0.1"))
    float PlayRate = 1.0f;

    // ===== 关键：打断规则 =====
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack|Interrupt")
    bool bUninterruptible = false; // 重攻击：不可打断（霸体）

    // 轻攻击：可按概率被打断（0=永不，1=必定）
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack|Interrupt", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InterruptChance = 0.6f;

    // 遭遇“重受击”时的打断概率（可让重受击更容易打断轻攻击）
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack|Interrupt", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InterruptChanceOnHeavyHit = 1.0f;

    // ===== 攻击轻重（用于技能/霸体/受击反馈扩展）=====
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack")
    EBMEnemyAttackWeight AttackWeight = EBMEnemyAttackWeight::Light;

    // ===== 工程参数：出手时的移动/转向策略（可按招式定制）=====
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack|Motion")
    bool bStopPathFollowingOnEnter = true; // StopMovement（保留惯性）

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack|Motion")
    bool bFaceTargetOnEnter = true;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack|HitBox")
    TArray<FName> HitBoxNames;

    // 这个攻击窗口的参数（倍率/去重/反馈覆写等）
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack|HitBox")
    FBMHitBoxActivationParams HitBoxParams;
};

USTRUCT(BlueprintType)
struct FBMPlayerComboStep
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FName Id = NAME_None;

    UPROPERTY(EditAnywhere)
    TObjectPtr<UAnimSequence> Anim = nullptr;

    UPROPERTY(EditAnywhere)
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere)
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere)
    float MaxPlayTime = -1.0f;

    // 连击窗口 = 本段结束前最后 LinkWindowSeconds 秒内按键有效
    UPROPERTY(EditAnywhere)
    float LinkWindowSeconds = 0.20f;

    
    UPROPERTY(EditAnywhere)
    float LinkWindowEndOffset = 0.02f;

    // 命中窗口
    UPROPERTY(EditAnywhere)
    TArray<FName> HitBoxNames;

    UPROPERTY(EditAnywhere)
    FBMHitBoxActivationParams HitBoxParams;

    // 打断属性
    UPROPERTY(EditAnywhere)
    bool bUninterruptible = false;

    UPROPERTY(EditAnywhere)
    float InterruptChance = 0.65f;

    UPROPERTY(EditAnywhere)
    float InterruptChanceOnHeavyHit = 1.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Combo|Recover")
    TObjectPtr<UAnimSequence> RecoverAnim = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Combo|Recover")
    float RecoverPlayRate = 1.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Combo|Recover")
    float RecoverStartTime = 0.0f;

    // <=0 表示播到结尾
    UPROPERTY(EditAnywhere, Category = "BM|Combo|Recover")
    float RecoverMaxPlayTime = -1.0f;
};

USTRUCT(BlueprintType)
struct FBMPlayerAttackSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack")
    FName Id = NAME_None;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack")
    TObjectPtr<UAnimSequence> Anim = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack", meta = (ClampMin = "0.01"))
    float Weight = 1.0f;

    // 动画播放控制
    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack", meta = (ClampMin = "0.01"))
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack", meta = (ClampMin = "0.0"))
    float StartTime = 0.0f;

    // <=0 表示播完整（从 StartTime 到结尾）
    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack")
    float MaxPlayTime = -1.0f;

    // 这段招式应打开哪个 HitBox（由动画 Notify/AnimEvent 调用 ActivateHitBox）
    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack")
    EBMHitBoxType HitBoxType = EBMHitBoxType::LightAttack;

    // 冷却
    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack", meta = (ClampMin = "0.0"))
    float Cooldown = 0.0f;

    // ===== 打断规则 =====
    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack|Interrupt")
    bool bUninterruptible = false;

    // 被“轻受击”打断概率（0=永不，1=必定）
    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack|Interrupt", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InterruptChance = 0.6f;

    // 被“重受击”打断概率
    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack|Interrupt", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InterruptChanceOnHeavyHit = 1.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack|HitBox")
    TArray<FName> HitBoxNames;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Attack|HitBox")
    FBMHitBoxActivationParams HitBoxParams;

};

USTRUCT(BlueprintType)
struct FBMPlayerSkillSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    EBMCombatAction Action = EBMCombatAction::Skill1;

    // 释放该技能需要消耗的耐力
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
    float StaminaCost = 0.f;

    UPROPERTY(EditAnywhere)
    FBMPlayerAttackSpec Spec;
};


USTRUCT(BlueprintType)
struct FBMDodgeSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    FName Id = TEXT("Dodge");          // 冷却 Key（每个角色自己一份 CombatComponent，所以不会互相影响）

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    TObjectPtr<UAnimSequence> Anim = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float MaxPlayTime = -1.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float Cooldown = 1.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float MoveSpeed = 900.f;           // 进入时沿锁定方向推一个速度
};

/**
 * 存档：背包物品条目
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
 * 存档：基础元信息（可用于"读取存档列表"UI展示）
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
 * 存档槽位信息（用于UI显示存档列表）
 */
USTRUCT(BlueprintType)
struct FBMSaveSlotInfo
{
    GENERATED_BODY()

    /** 槽位编号 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
    int32 SlotNumber = -1;

    /** 存档是否存在 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
    bool bExists = false;

    /** 存档元信息（仅在存档存在时有效） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
    FBMSaveMeta Meta;
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
    static const FName Dodge = TEXT("Dodge"); 
    static const FName Attack = TEXT("Attack");
    static const FName SkillCast = TEXT("SkillCast");
    static const FName Hit = TEXT("Hit");
    static const FName Death = TEXT("Death");

}


namespace BMEnemyStateNames
{
    static const FName Idle = TEXT("Enemy.Idle");
    static const FName Patrol = TEXT("Enemy.Patrol");
    static const FName Chase = TEXT("Enemy.Chase");
    static const FName Dodge = TEXT("Dodge");
    static const FName Attack = TEXT("Enemy.Attack");
    static const FName Hit = TEXT("Enemy.Hit");
    static const FName Death = TEXT("Enemy.Death");
    static const FName PhaseChange = TEXT("PhaseChange");
}

namespace BMCombatUtils
{
    inline bool IsHeavyIncoming(const FBMDamageInfo& Info)
    {
        switch (Info.HitReaction)
        {
            case EBMHitReaction::Heavy:
            case EBMHitReaction::KnockDown:
            case EBMHitReaction::Airborne:
                return true;
            default:
                return false;
        }
    }

    inline bool IsSkillAction(EBMCombatAction A)
    {
        return A == EBMCombatAction::Skill1
            || A == EBMCombatAction::Skill2
            || A == EBMCombatAction::Skill3;
    }
}