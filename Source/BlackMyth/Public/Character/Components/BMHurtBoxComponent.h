#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMHurtBoxComponent.generated.h"

class UPrimitiveComponent;

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMHurtBoxComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMHurtBoxComponent();

    float GetDamageMultiplier() const { return DamageMultiplier; }

    bool IsBoundTo(const UPrimitiveComponent* InComp) const { return BoundComponent == InComp; }
    void BindTo(UPrimitiveComponent* InComp) { BoundComponent = InComp; }

    // 核心：统一在这里对伤害信息进行“倍率/弱抗/受击反馈”等调整
    virtual void ModifyIncomingDamage(FBMDamageInfo& InOutInfo) const;

public:
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox")
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, Category = "BM|HurtBox")
    TArray<EBMElementType> WeaknessTypes;

    UPROPERTY(EditAnywhere, Category = "BM|HurtBox")
    TArray<EBMElementType> ResistanceTypes;

    // 可选：不手动 BindTo 就用名字自动找
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox")
    FName BoundComponentName = NAME_None;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<UPrimitiveComponent> BoundComponent = nullptr;
};
