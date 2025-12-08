#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMHitBoxComponent.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class ABMCharacterBase;

DECLARE_LOG_CATEGORY_EXTERN(LogBMHitBox, Log, All);

// 命中事件：把最终 FBMDamageInfo 广播出去（可做特效/飘字/音效）
DECLARE_MULTICAST_DELEGATE_ThreeParams(FBMOnHitLanded, ABMCharacterBase* /*Instigator*/, ABMCharacterBase* /*Victim*/, const FBMDamageInfo& /*FinalInfo*/);

USTRUCT()
struct FBMHitBoxConfig
{
    GENERATED_BODY()

    float BaseDamage = 10.f;
    EBMDamageType DamageType = EBMDamageType::Melee;
    EBMElementType ElementType = EBMElementType::Physical;
    EBMHitReaction DefaultReaction = EBMHitReaction::Light;
    float KnockbackStrength = 0.f;
};

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMHitBoxComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMHitBoxComponent();

    void ActivateHitBox(EBMHitBoxType Type);
    void DeactivateHitBox();
    void ResetHitList();

    // 配置接口（你可以在构造里给不同 Type 配不同伤害）
    void SetConfig(EBMHitBoxType Type, const FBMHitBoxConfig& InConfig);

    FBMOnHitLanded OnHitLanded;

protected:
    virtual void BeginPlay() override;

private:
    ABMCharacterBase* GetOwnerCharacter() const;
    void EnsureDefaultHitBoxCreated();

    const FBMHitBoxConfig& GetConfigForType(EBMHitBoxType Type) const;

    UFUNCTION()
    void OnHitBoxOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

private:
    UPROPERTY()
    TMap<EBMHitBoxType, TObjectPtr<UBoxComponent>> HitBoxes;

    UPROPERTY()
    TObjectPtr<UBoxComponent> ActiveBox = nullptr;

    EBMHitBoxType ActiveType = EBMHitBoxType::Default;

    TMap<EBMHitBoxType, FBMHitBoxConfig> Configs;

    // 单次挥砍/技能窗口内：同一目标只结算一次
    TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;
};
