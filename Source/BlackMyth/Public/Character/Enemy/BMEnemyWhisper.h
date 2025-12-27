#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "BMEnemyWhisper.generated.h"

class USkeletalMesh;
class UAnimSequence;
class UBMHurtBoxComponent;

/**
 * Whisper 敌人类
 *
 * 继承自 ABMEnemyBase，提供 Whisper 敌人特有功能：
 * - 2轻1重攻击招式组合
 * - 三个 HurtBox 组件（身体、腹部、头部）
 * - 高闪避概率和快速闪避动画
 * - 可配置的掉落物品表
 *
 * Whisper 作为灵活型敌人，拥有更高的闪避能力和快速攻击节奏
 */
UCLASS()
class BLACKMYTH_API ABMEnemyWhisper : public ABMEnemyBase
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 创建 HurtBox 组件并初始化 Whisper 特有参数
     */
    ABMEnemyWhisper();
    
    /**
     * 开始游戏生命周期
     *
     * 应用资产、构建攻击规格、HitBox、HurtBox 和掉落物表
     */
    virtual void BeginPlay() override;
    
    /**
     * 获取敌人数据 ID
     *
     * @return 返回 "EnemyWhisper" 用于 DataTable 查找
     */
    virtual FName GetEnemyDataID() const override { return FName("EnemyWhisper"); }

protected:
    /**
     * 应用配置的资产
     *
     * 加载并应用骨骼网格和动画资产
     */
    void ApplyConfiguredAssets();
    
    /**
     * 构建攻击规格
     *
     * 根据配置的攻击动画创建攻击规格列表（2轻1重）
     */
    void BuildAttackSpecs();
    
    /**
     * 构建 HitBox
     *
     * 为攻击动画配置 HitBox 组件
     */
    void BuildHitBoxes();
    
    /**
     * 构建 HurtBox
     *
     * 配置身体、腹部和头部的 HurtBox 受击判定
     */
    void BuildHurtBoxes();
    
    /**
     * 构建掉落物品表
     *
     * 配置敌人死亡后的掉落物品列表
     */
    void BuildLootTable();
    
    /**
     * 播放闪避动画一次
     *
     * Whisper 支持快速闪避（播放速率2.0）
     *
     * @return 动画时长（秒）
     */
    virtual float PlayDodgeOnce() override;

protected:
    // ===== 基础资产 =====
    
    /** 骨骼网格资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<USkeletalMesh> MeshAsset;

    /** 待机动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimIdleAsset;

    /** 行走动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimWalkAsset;

    /** 奔跑动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimRunAsset;

    /** 轻度受击动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitLightAsset;

    /** 重度受击动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitHeavyAsset;

    /** 死亡动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDeathAsset;

    /** 闪避动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDodgeAsset;

    // ===== 攻击动画资产（2轻1重）=====
    
    /** 轻攻击1动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackLight1Asset;

    /** 轻攻击2动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackLight2Asset;

    /** 重攻击动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackHeavyAsset;

protected:
    // ===== HurtBox 组件 =====
    
    /** 身体 HurtBox 组件 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Whisper|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtBody = nullptr;

    /** 腹部 HurtBox 组件 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Whisper|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtAbdomen = nullptr;

    /** 头部 HurtBox 组件 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Whisper|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtHead = nullptr;

protected:
    // ===== Whisper 可调参数 =====
    
    /** Whisper 警戒范围 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Tuning")
    float WhisperAggroRange = 900.f;

    /** Whisper 巡逻半径 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Tuning")
    float WhisperPatrolRadius = 500.f;

    /** Whisper 巡逻速度 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Tuning")
    float WhisperPatrolSpeed = 220.f;

    /** Whisper 追击速度 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Tuning")
    float WhisperChaseSpeed = 420.f;

    /** Whisper 基础伤害 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Tuning")
    float WhisperBaseDamage = 12.f;

    // ===== 闪避参数 =====
    
    /** Whisper 闪避距离 */
    UPROPERTY(EditAnywhere, Category = "BM|Whisper|Dodge")
    float WhisperDodgeDistance = 220.f;

    /** Whisper 受击闪避概率*/
    UPROPERTY(EditAnywhere, Category = "BM|Whisper|Dodge")
    float WhisperDodgeOnHitChance = 0.50f;

    /** Whisper 闪避冷却时间 */
    UPROPERTY(EditAnywhere, Category = "BM|Whisper|Dodge")
    float WhisperDodgeCooldown = 2.0f;

    /** Whisper 闪避动画播放速率*/
    UPROPERTY(EditAnywhere, Category = "BM|Whisper|Dodge")
    float WhisperDodgePlayRate = 2.0f;

    /** Whisper 闪避冷却键名 */
    UPROPERTY(EditAnywhere, Category = "BM|Whisper|Dodge")
    FName WhisperDodgeCooldownKey = TEXT("Whisper_Dodge");
};
