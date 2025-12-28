#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "BMEnemyDummy.generated.h"

class USkeletalMesh;
class UAnimSequence;
class UBMHurtBoxComponent;

/**
 * 基础小怪（Dummy）
 * - 复用 ABMEnemyBase 的 FSM（Idle/Patrol/Chase/Attack/Hit/Death）
 * - 只负责配置
 */
UCLASS()
class BLACKMYTH_API ABMEnemyDummy : public ABMEnemyBase
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 创建 HurtBox 组件并初始化 Dummy 特有参数
     */
    ABMEnemyDummy();
    
    /**
     * 开始游戏生命周期
     *
     * 应用资产、构建攻击规格、HitBox、HurtBox 和掉落物表
     */
    virtual void BeginPlay() override;
    
    /**
     * 获取敌人数据 ID
     *
     * @return 返回 "EnemyDummy" 用于 DataTable 查找
     */
    virtual FName GetEnemyDataID() const override { return FName("EnemyDummy"); }

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
     * Dummy 敌人支持闪避
     *
     * @return 动画时长（秒）
     */
    virtual float PlayDodgeOnce() override;

protected:
    // ===== 资产 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets")
    TSoftObjectPtr<USkeletalMesh> MeshAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets")
    TSoftObjectPtr<UAnimSequence> AnimIdleAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets")
    TSoftObjectPtr<UAnimSequence> AnimWalkAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets")
    TSoftObjectPtr<UAnimSequence> AnimRunAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitLightAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitHeavyAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDeathAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDodgeAsset;

    // ===== 攻击动画：1轻2重 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackLightAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackHeavy1Asset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackHeavy2Asset;

protected:
    // ===== HurtBox =====
    UPROPERTY(VisibleAnywhere, Category = "BM|Dummy|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtBody = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "BM|Dummy|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtHead = nullptr;

    // ===== 可调参数 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Tuning")
    float DummyAggroRange = 900.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Tuning")
    float DummyPatrolRadius = 500.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Tuning")
    float DummyPatrolSpeed = 220.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Tuning")
    float DummyChaseSpeed = 420.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Dummy|Tuning")
    float DummyBaseDamage = 12.f;

    UPROPERTY(EditAnywhere, Category = "BM|Dummy|Dodge")
    float DummyDodgeDistance = 420.f;

    UPROPERTY(EditAnywhere, Category = "BM|Dummy|Dodge")
    float DummyDodgeOnHitChance = 0.3f;

    UPROPERTY(EditAnywhere, Category = "BM|Dummy|Dodge")
    float DummyDodgeCooldown = 2.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Loot", meta = (ClampMin = "0"))
    int32 DummyCurrencyDropMin = 200;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Loot", meta = (ClampMin = "0"))
    int32 DummyCurrencyDropMax = 300;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Loot", meta = (ClampMin = "0.0"))
    float DummyExpDropMin = 100.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Loot", meta = (ClampMin = "0.0"))
    float DummyExpDropMax = 300.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Dummy|Dodge")
    FName DummyDodgeCooldownKey = TEXT("Dummy_Dodge");
};
