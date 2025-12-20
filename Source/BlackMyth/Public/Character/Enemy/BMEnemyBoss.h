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

protected:
    void ApplyConfiguredAssets();
    void ApplyBossBodyTuning();
    void BuildAttackSpecs();
    void BuildHitBoxes();
    void BuildHurtBoxes();
    virtual float PlayDodgeOnce() override;

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
};
