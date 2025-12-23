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
 * - 只负责“配置”：网格/动画/攻击Spec/HitBox/HurtBox
 */
UCLASS()
class BLACKMYTH_API ABMEnemyDummy : public ABMEnemyBase
{
    GENERATED_BODY()

public:
    ABMEnemyDummy();
    virtual void BeginPlay() override;

protected:
    void ApplyConfiguredAssets();
    void BuildAttackSpecs();
    void BuildHitBoxes();
    void BuildHurtBoxes();
	void BuildLootTable();
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
