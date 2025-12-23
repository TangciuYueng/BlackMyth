#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "BMEnemyDemon.generated.h"

class USkeletalMesh;
class UAnimSequence;
class UBMHurtBoxComponent;

UCLASS()
class BLACKMYTH_API ABMEnemyDemon : public ABMEnemyBase
{
    GENERATED_BODY()

public:
    ABMEnemyDemon();
    virtual void BeginPlay() override;

protected:
    void ApplyConfiguredAssets();
    void BuildAttackSpecs();
    void BuildHitBoxes();
    void BuildHurtBoxes();
    virtual float PlayDodgeOnce() override;

protected:
    // ===== 资产 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<USkeletalMesh> MeshAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimIdleAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimWalkAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimRunAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitLightAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitHeavyAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDeathAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDodgeAsset;

    // ===== 攻击动画：1轻2重 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackLightAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackHeavy1Asset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackHeavy2Asset;

protected:
    // ===== HurtBox =====
    UPROPERTY(VisibleAnywhere, Category = "BM|Demon|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtBody = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "BM|Demon|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtHead = nullptr;

    // ===== 可调参数 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Tuning")
    float DemonAggroRange = 900.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Tuning")
    float DemonPatrolRadius = 500.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Tuning")
    float DemonPatrolSpeed = 220.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Tuning")
    float DemonChaseSpeed = 420.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Demon|Tuning")
    float DemonBaseDamage = 12.f;

    UPROPERTY(EditAnywhere, Category = "BM|Demon|Dodge")
    float DemonDodgeDistance = 420.f;

    UPROPERTY(EditAnywhere, Category = "BM|Demon|Dodge")
    float DemonDodgeOnHitChance = 0.3f;

    UPROPERTY(EditAnywhere, Category = "BM|Demon|Dodge")
    float DemonDodgeCooldown = 2.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Demon|Dodge")
    FName DemonDodgeCooldownKey = TEXT("Demon_Dodge");
};
