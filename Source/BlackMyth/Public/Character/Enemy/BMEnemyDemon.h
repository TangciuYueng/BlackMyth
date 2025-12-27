#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "BMEnemyDemon.generated.h"

class USkeletalMesh;
class UAnimSequence;
class UBMHurtBoxComponent;

/**
 * 恶魔敌人类
 *
 * 继承自 ABMEnemyBase，提供恶魔敌人特有功能：
 * - 配置式资产加载（骨骼网格和动画）
 * - 1轻2重攻击招式组合
 * - 身体和头部 HurtBox 配置
 * - 可自定义掉落物品表
 * - 可调整的 AI 参数和闪避参数
 */
UCLASS()
class BLACKMYTH_API ABMEnemyDemon : public ABMEnemyBase
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 创建 HurtBox 组件并初始化恶魔敌人参数
     */
    ABMEnemyDemon();
    
    /**
     * 开始游戏生命周期
     *
     * 应用资产、构建攻击规格、HitBox、HurtBox 和掉落物表
     */
    virtual void BeginPlay() override;
    
    /**
     * 获取敌人数据 ID
     *
     * @return 返回 EnemyDemon 用于 DataTable 查找
     */
    virtual FName GetEnemyDataID() const override { return FName("EnemyDemon"); }

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
     * 根据配置的攻击动画创建攻击规格列表（1轻2重）
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
     * 配置身体和头部的 HurtBox 受击判定
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
     * 恶魔敌人支持闪避
     *
     * @return 动画时长（秒）
     */
    virtual float PlayDodgeOnce() override;

protected:
    // ===== 基础资产 =====
    
    /** 骨骼网格资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<USkeletalMesh> MeshAsset;

    /** 待机动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimIdleAsset;

    /** 行走动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimWalkAsset;

    /** 奔跑动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimRunAsset;

    /** 轻度受击动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitLightAsset;

    /** 重度受击动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitHeavyAsset;

    /** 死亡动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDeathAsset;

    /** 闪避动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDodgeAsset;

    // ===== 攻击动画资产（1轻2重）=====
    
    /** 轻攻击动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackLightAsset;

    /** 重攻击1动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackHeavy1Asset;

    /** 重攻击2动画资产 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackHeavy2Asset;

protected:
    // ===== HurtBox 组件 =====
    
    /** 身体 HurtBox 组件 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Demon|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtBody = nullptr;

    /** 头部 HurtBox 组件 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Demon|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtHead = nullptr;

    // ===== 恶魔可调参数 =====
    
    /** 恶魔警戒范围 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Tuning")
    float DemonAggroRange = 900.f;

    /** 恶魔巡逻半径 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Tuning")
    float DemonPatrolRadius = 500.f;

    /** 恶魔巡逻速度 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Tuning")
    float DemonPatrolSpeed = 220.f;

    /** 恶魔追击速度 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Tuning")
    float DemonChaseSpeed = 420.f;

    /** 恶魔闪避距离 */
    UPROPERTY(EditAnywhere, Category = "BM|Demon|Dodge")
    float DemonDodgeDistance = 420.f;

    /** 恶魔受击闪避概率 */
    UPROPERTY(EditAnywhere, Category = "BM|Demon|Dodge")
    float DemonDodgeOnHitChance = 0.3f;

    /** 恶魔闪避冷却时间 */
    UPROPERTY(EditAnywhere, Category = "BM|Demon|Dodge")
    float DemonDodgeCooldown = 2.0f;

    /** 恶魔闪避冷却键名 */
    UPROPERTY(EditAnywhere, Category = "BM|Demon|Dodge")
    FName DemonDodgeCooldownKey = TEXT("Demon_Dodge");
};
