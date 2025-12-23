#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "BMEnemyBoss.generated.h"

class USkeletalMesh;
class UAnimSequence;
class UBMHurtBoxComponent;

UCLASS()
class BLACKMYTH_API ABMEnemyBoss : public ABMEnemyBase
{
    GENERATED_BODY()

public:
    ABMEnemyBoss();
    virtual void BeginPlay() override;
    void EnterPhase2();
    void SetPhaseTransition(bool bIn) { bInPhaseTransition = bIn; }
    bool IsInPhaseTransition() const { return bInPhaseTransition; }
    float PlayEnergizeOnce();
    float PlayDeathReverseOnce(float ReversePlayRate = 1.0f, float ReverseMaxTime = -1.0f);
    float GetPhase2DeathHoldSeconds() const { return Phase2DeathHoldSeconds; }
    float GetPhase2DeathReversePlayRate() const { return Phase2DeathReversePlayRate; }
    float GetPhase2DeathReverseMaxTime() const { return Phase2DeathReverseMaxTime; }
    virtual void SetAlertState(bool bAlert) override;

protected:
    void ApplyConfiguredAssets();
    void ApplyBossBodyTuning();
    void BuildAttackSpecs();
    void BuildHitBoxes();
    void BuildHurtBoxes();
    virtual float PlayDodgeOnce() override;
    virtual void HandleDeath(const FBMDamageInfo& LastHitInfo) override;
    virtual bool CanBeDamagedBy(const FBMDamageInfo& Info) const override;
    void AddPhase2AttackSpecs();     // 只追加二阶段新招式
    void ApplyPhase2Tuning();        // 加血、加伤害等

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
    float Phase2MaxHP = 300.f;               

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Phase2")
    float Phase2BaseDamage = 32.f;           

protected:
    // ===== HurtBox（配置式组件）=====
    UPROPERTY(VisibleAnywhere, Category = "BM|Boss|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtBody = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "BM|Boss|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtAbdomen = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "BM|Boss|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtHead = nullptr;

protected:
    // ===== 可调参数 =====
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack")
    float BossGlobalAttackInterval = 2.5f;

    // 全局间隔的随机浮动范围
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack")
    float BossGlobalAttackIntervalDeviation = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Tuning")
    float BossAggroRange = 800.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Tuning")
    float BossPatrolRadius = 650.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Tuning")
    float BossPatrolSpeed = 180.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Tuning")
    float BossChaseSpeed = 320.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Tuning")
    float BossBaseDamage = 22.f;

    UPROPERTY(EditAnywhere, Category = "BM|Boss|Dodge")
    float BossDodgeDistance = 360.f;

    UPROPERTY(EditAnywhere, Category = "BM|Boss|Dodge")
    float BossDodgeOnHitChance = 0.18f;

    UPROPERTY(EditAnywhere, Category = "BM|Boss|Dodge")
    float BossDodgeCooldown = 3.2f;

    UPROPERTY(EditAnywhere, Category = "BM|Boss|Dodge")
    FName BossDodgeCooldownKey = TEXT("Boss_Dodge");

private:
    bool bReviveUsed = false;        // 是否已经用过一次复活
    bool bIsPhase2 = false;          // 当前是否二阶段
    bool bInPhaseTransition = false; // 过渡期
    // Death 播完后停留多久
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Phase2|Transition")
    float Phase2DeathHoldSeconds = 3.0f;

    // 倒放播放率
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Phase2|Transition")
    float Phase2DeathReversePlayRate = 0.7f;

    // 倒放时长
    UPROPERTY(EditDefaultsOnly, Category = "BM|Boss|Phase2|Transition")
    float Phase2DeathReverseMaxTime = -1.0f;
    UPROPERTY(Transient)
    bool bBossAlertMusicStarted = false;

    UPROPERTY(EditAnywhere, Category = "BM|Boss|Phase")
    int32 BossPhase = 1; // 1: 第一阶段, 2: 第二阶段

    // 第一阶段死亡到播放 boss2 的延迟（秒）
    UPROPERTY(EditAnywhere, Category = "BM|Boss|Phase")
    float Phase1To2MusicDelaySeconds = 6.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "BM|Boss|Phase")
    void SetBossPhase(int32 InPhase) { BossPhase = InPhase; }
    UFUNCTION(BlueprintCallable, Category = "BM|Boss|Phase")
    int32 GetBossPhase() const { return BossPhase; }
};
