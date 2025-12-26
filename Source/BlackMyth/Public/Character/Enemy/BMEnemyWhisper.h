#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "BMEnemyWhisper.generated.h"

class USkeletalMesh;
class UAnimSequence;
class UBMHurtBoxComponent;

/**
 * Whisper
 * - 两个轻攻击 + 一个重攻击
 */
UCLASS()
class BLACKMYTH_API ABMEnemyWhisper : public ABMEnemyBase
{
    GENERATED_BODY()

public:
ABMEnemyWhisper();
virtual void BeginPlay() override;
    
// Override to return enemy data identifier
virtual FName GetEnemyDataID() const override { return FName("EnemyWhisper"); }

protected:
    void ApplyConfiguredAssets();
    void BuildAttackSpecs();
    void BuildHitBoxes();
    void BuildHurtBoxes();
    void BuildLootTable();
    virtual float PlayDodgeOnce() override;

protected:
    // ===== 资产 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<USkeletalMesh> MeshAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimIdleAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimWalkAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimRunAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitLightAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimHitHeavyAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDeathAsset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets")
    TSoftObjectPtr<UAnimSequence> AnimDodgeAsset;

    // ===== 攻击动画：2轻1重 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackLight1Asset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackLight2Asset;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Assets|Attack")
    TSoftObjectPtr<UAnimSequence> AttackHeavyAsset;

protected:
    // ===== HurtBox =====
    UPROPERTY(VisibleAnywhere, Category = "BM|Whisper|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtBody = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "BM|Whisper|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtAbdomen = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "BM|Whisper|Components")
    TObjectPtr<UBMHurtBoxComponent> HurtHead = nullptr;

protected:
    // ===== 可调参数 =====
    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Tuning")
    float WhisperAggroRange = 900.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Tuning")
    float WhisperPatrolRadius = 500.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Tuning")
    float WhisperPatrolSpeed = 220.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Tuning")
    float WhisperChaseSpeed = 420.f;

    UPROPERTY(EditDefaultsOnly, Category = "BM|Whisper|Tuning")
    float WhisperBaseDamage = 12.f;

    // ===== Dodge =====
    UPROPERTY(EditAnywhere, Category = "BM|Whisper|Dodge")
    float WhisperDodgeDistance = 220.f;

    UPROPERTY(EditAnywhere, Category = "BM|Whisper|Dodge")
    float WhisperDodgeOnHitChance = 0.50f;

    UPROPERTY(EditAnywhere, Category = "BM|Whisper|Dodge")
    float WhisperDodgeCooldown = 2.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Whisper|Dodge")
    float WhisperDodgePlayRate = 2.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Whisper|Dodge")
    FName WhisperDodgeCooldownKey = TEXT("Whisper_Dodge");
};
