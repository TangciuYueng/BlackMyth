#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "BMEnemyBoss.generated.h"

class USkeletalMesh;
class UAnimSequence;
class UBMHurtBoxComponent;

/**
 * Boss 敌人类
 *
 * 继承自 ABMEnemyBase，提供 Boss 特有功能：
 * - 两阶段战斗系统（死亡后复活进入二阶段）
 * - 自定义体型和碰撞参数
 * - 多个可配置的 HurtBox 组件（身体、腹部、头部）
 * - 阶段转换动画和过渡逻辑
 * - 二阶段新增攻击招式和属性提升
 * - 禁用悬浮血条
 */
UCLASS()
class BLACKMYTH_API ABMEnemyBoss : public ABMEnemyBase
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 创建 HurtBox 组件并初始化 Boss 特有参数
     */
    ABMEnemyBoss();
    
    /**
     * 开始游戏生命周期
     *
     * 应用资产、体型调整、构建攻击规格和 HitBox/HurtBox
     */
    virtual void BeginPlay() override;
    
    /**
     * 获取敌人数据 ID
     *
     * @return 返回 EnemyBoss 用于 DataTable 查找
     */
    virtual FName GetEnemyDataID() const override { return FName("EnemyBoss"); }
    
    /**
     * 进入二阶段
     *
     * 提升属性、添加新招式并应用二阶段调整
     */
    void EnterPhase2();
    
    /**
     * 设置阶段转换状态
     *
     * @param bIn true 进入转换，false 结束转换
     */
    void SetPhaseTransition(bool bIn) { bInPhaseTransition = bIn; }
    
    /**
     * 查询是否处于阶段转换中
     *
     * @return 处于转换返回 true
     */
    bool IsInPhaseTransition() const { return bInPhaseTransition; }
    
    /**
     * 播放蓄力动画一次
     *
     * @return 动画时长（秒）
     */
    float PlayEnergizeOnce();
    
    /**
     * 播放死亡动画倒放一次
     *
     * @param ReversePlayRate 倒放速率
     * @param ReverseMaxTime 倒放最大时长
     * @return 动画时长（秒）
     */
    float PlayDeathReverseOnce(float ReversePlayRate = 1.0f, float ReverseMaxTime = -1.0f);
    
    /** 获取二阶段死亡停留时长 */
    float GetPhase2DeathHoldSeconds() const { return Phase2DeathHoldSeconds; }
    
    /** 获取二阶段死亡倒放速率 */
    float GetPhase2DeathReversePlayRate() const { return Phase2DeathReversePlayRate; }
    
    /** 获取二阶段死亡倒放最大时长 */
    float GetPhase2DeathReverseMaxTime() const { return Phase2DeathReverseMaxTime; }
    
    /**
     * 设置警戒状态
     *
     * 重写以播放 Boss 专属音乐
     *
     * @param bAlert true 进入警戒，false 解除警戒
     */
    virtual void SetAlertState(bool bAlert) override;

    /**
     * 是否显示悬浮血条
     *
     * Boss 禁用悬浮血条，使用专用 Boss 血条
     *
     * @return 始终返回 false
     */
    virtual bool ShouldShowFloatingHealthBar() const override { return false; }

protected:
/**
 * 应用配置的资产
 *
 * 加载并应用骨骼网格和动画资产
 */
void ApplyConfiguredAssets();
    
/**
 * 应用 Boss 体型调整
 *
 * 设置胶囊体大小、网格缩放和偏移
 */
void ApplyBossBodyTuning();
    
/**
 * 构建攻击规格
 *
 * 根据配置的攻击动画创建一阶段攻击规格列表
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
 * 配置身体、腹部、头部的 HurtBox 受击判定
 */
void BuildHurtBoxes();
    
/**
 * 播放闪避动画一次
 *
 * Boss 禁用闪避，返回 0
 *
 * @return 始终返回 0
 */
virtual float PlayDodgeOnce() override;
    
/**
 * 处理死亡
 *
 * 一阶段死亡触发阶段转换，二阶段死亡正常处理
 *
 * @param LastHitInfo 致死伤害信息
 */
virtual void HandleDeath(const FBMDamageInfo& LastHitInfo) override;
    
/**
 * 判断是否可被该伤害击中
 *
 * 阶段转换期间无敌
 *
 * @param Info 伤害信息
 * @return 可被击中返回 true
 */
virtual bool CanBeDamagedBy(const FBMDamageInfo& Info) const override;
    
/**
 * 添加二阶段攻击规格
 *
 * 追加二阶段新招式到攻击列表
 */
void AddPhase2AttackSpecs();
    
/**
 * 应用二阶段调整
 *
 * 提升生命值和攻击力
 */
void ApplyPhase2Tuning();

protected:
    // ===== Boss 体型/碰撞调参 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Body")
    float BossMeshScale = 1.45f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Body")
    float BossCapsuleRadius = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Body")
    float BossCapsuleHalfHeight = 120.f;

    // Mesh 的基础偏移
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Body")
    float BaseMeshZOffset = -90.f;

    // ===== 资产 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets")
    TSoftObjectPtr<USkeletalMesh> MeshAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets")
    TSoftObjectPtr<UAnimSequence> AnimIdleAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets")
    TSoftObjectPtr<UAnimSequence> AnimWalkAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets")
    TSoftObjectPtr<UAnimSequence> AnimRunAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitLightAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitHeavyAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDeathAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDodgeAsset;

    // ===== 攻击动画 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackLight1Asset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackLight2Asset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackHeavy1Asset;

    // ===== 二阶段新增招式资源 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets|Attack|Phase2")
    TSoftObjectPtr<UAnimSequence> AttackPhase2Heavy2Asset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets|Attack|Phase2")
    TSoftObjectPtr<UAnimSequence> AttackPhase2Light3Asset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Assets")
    TSoftObjectPtr<UAnimSequence> AnimEnergizeAsset;

    UPROPERTY(Transient)
    TObjectPtr<UAnimSequence> AnimEnergize = nullptr;

    // ===== 二阶段调参 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Phase2")
    float Phase2MaxHP = 3000.f;               

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Phase2")
    float Phase2BaseDamage = 60.f;           

protected:
    // ===== HurtBox 组件 =====
    
    /** 身体 HurtBox 组件 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Boss|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtBody = nullptr;

    /** 腹部 HurtBox 组件 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Boss|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtAbdomen = nullptr;

    /** 头部 HurtBox 组件 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Boss|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtHead = nullptr;

protected:
    // ===== Boss 可调参数 =====
    
    /** Boss 全局攻击间隔（秒）*/
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack")
    float BossGlobalAttackInterval = 2.5f;

    /** Boss 全局攻击间隔随机浮动范围（秒）*/
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack")
    float BossGlobalAttackIntervalDeviation = 1.5f;

    /** Boss 警戒范围 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Tuning")
    float BossAggroRange = 800.f;

    /** Boss 巡逻半径 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Tuning")
    float BossPatrolRadius = 650.f;

    /** Boss 巡逻速度 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Tuning")
    float BossPatrolSpeed = 180.f;

    /** Boss 追击速度 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Tuning")
    float BossChaseSpeed = 320.f;

    /** Boss 基础伤害 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Tuning")
    float BossBaseDamage = 22.f;

    /** Boss 闪避距离 */
    UPROPERTY(EditAnywhere, Category = "BM|Boss|Dodge")
    float BossDodgeDistance = 360.f;

    /** Boss 受击闪避概率 */
    UPROPERTY(EditAnywhere, Category = "BM|Boss|Dodge")
    float BossDodgeOnHitChance = 0.18f;

    /** Boss 闪避冷却时间 */
    UPROPERTY(EditAnywhere, Category = "BM|Boss|Dodge")
    float BossDodgeCooldown = 3.2f;

    /** Boss 闪避冷却键名 */
    UPROPERTY(EditAnywhere, Category = "BM|Boss|Dodge")
    FName BossDodgeCooldownKey = TEXT("Boss_Dodge");

private:
    /** 是否已使用过一次复活 */
    bool bReviveUsed = false;
    
    /** 当前是否处于二阶段 */
    bool bIsPhase2 = false;
    
    /** 是否处于阶段转换期间 */
    bool bInPhaseTransition = false;
    
    /** 二阶段死亡动画播完后停留时长（秒）*/
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Phase2|Transition")
    float Phase2DeathHoldSeconds = 3.0f;

    /** 二阶段死亡倒放播放速率 */
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Phase2|Transition")
    float Phase2DeathReversePlayRate = 0.7f;

    /** 二阶段死亡倒放时长（<=0 表示播完）*/
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Phase2|Transition")
    float Phase2DeathReverseMaxTime = -1.0f;
    
    /** Boss 警戒音乐是否已开始播放 */
    UPROPERTY(Transient)
    bool bBossAlertMusicStarted = false;

    /** Boss 当前阶段（1: 一阶段, 2: 二阶段）*/
    UPROPERTY(EditAnywhere, Category = "BM|Boss|Phase")
    int32 BossPhase = 1;

    /** 一阶段死亡到播放二阶段音乐的延迟（秒）*/
    UPROPERTY(EditAnywhere, Category = "BM|Boss|Phase")
    float Phase1To2MusicDelaySeconds = 6.0f;

public:
    /**
     * 设置 Boss 阶段
     *
     * @param InPhase 阶段值（1 或 2）
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Boss|Phase")
    void SetBossPhase(int32 InPhase) { BossPhase = InPhase; }
    
    /**
     * 获取 Boss 当前阶段
     *
     * @return 当前阶段值（1 或 2）
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Boss|Phase")
    int32 GetBossPhase() const { return BossPhase; }
};
