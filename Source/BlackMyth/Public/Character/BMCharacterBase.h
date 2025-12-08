#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/BMTypes.h"
#include "BMCharacterBase.generated.h"

class UBMStatsComponent;
class UBMCombatComponent;
class UBMStateMachineComponent;
class UBMAnimEventComponent;
class UBMHitBoxComponent;
class UBMHurtBoxComponent;
class UAnimMontage;
class UPrimitiveComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogBMCharacter, Log, All);

// 纯C++事件：把最终 DamageInfo 带出去（UI/飘字/事件总线都能复用）
DECLARE_MULTICAST_DELEGATE_TwoParams(FBMOnCharacterDamaged, class ABMCharacterBase* /*Victim*/, const FBMDamageInfo& /*FinalInfo*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FBMOnCharacterDied, class ABMCharacterBase* /*Victim*/, const FBMDamageInfo& /*LastHitInfo*/);

UCLASS(Abstract)
class BLACKMYTH_API ABMCharacterBase : public ACharacter
{
    GENERATED_BODY()

public:
    ABMCharacterBase();

    virtual void Tick(float DeltaSeconds) override;
    virtual void BeginPlay() override;

    // 统一伤害入口：必须用 FBMDamageInfo（引用回填）
    virtual float TakeDamageFromHit(FBMDamageInfo& InOutInfo);

    virtual FVector GetForwardVector() const;

    // 组件访问
    UBMStatsComponent* GetStats() const { return Stats; }
    UBMCombatComponent* GetCombat() const { return Combat; }
    UBMStateMachineComponent* GetFSM() const { return FSM; }
    UBMAnimEventComponent* GetAnimEvent() const { return AnimEvent; }
    UBMHitBoxComponent* GetHitBox() const { return HitBox; }

    // 事件
    FBMOnCharacterDamaged OnCharacterDamaged;
    FBMOnCharacterDied OnCharacterDied;

    // 基础身份（便于做友伤/阵营）
    UPROPERTY(EditAnywhere, Category = "BM|Identity")
    EBMTeam Team = EBMTeam::Neutral;

    UPROPERTY(EditAnywhere, Category = "BM|Identity")
    EBMCharacterType CharacterType = EBMCharacterType::Enemy;

protected:
    // === 多态钩子：派生类 override 改大行为 ===
    virtual bool CanBeDamagedBy(const FBMDamageInfo& Info) const;
    virtual void HandleDamageTaken(const FBMDamageInfo& FinalInfo);
    virtual void HandleDeath(const FBMDamageInfo& LastHitInfo);

    // 可选：受击/死亡 montage（派生类按反应类型返回不同动画）
    virtual UAnimMontage* GetHitReactMontage(EBMHitReaction Reaction) const { (void)Reaction; return nullptr; }
    virtual UAnimMontage* GetDeathMontage() const { return nullptr; }

    // HurtBox：倍率查询
    virtual float GetDamageMultiplierForComponent(const UPrimitiveComponent* HitComponent) const;

protected:
    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMStatsComponent> Stats;

    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMCombatComponent> Combat;

    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMStateMachineComponent> FSM;

    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMAnimEventComponent> AnimEvent;

    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMHitBoxComponent> HitBox;

    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TArray<TObjectPtr<UBMHurtBoxComponent>> HurtBoxes;

private:
    void CacheHurtBoxes();

    void HandleStatsDeath(AActor* Killer);

private:
    // 保存最近一次真正造成扣血的伤害信息（死亡回调/事件用）
    FBMDamageInfo LastAppliedDamageInfo;
};
