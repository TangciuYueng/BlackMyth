#include "Character/Components/BMHurtBoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

UBMHurtBoxComponent::UBMHurtBoxComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UBMHurtBoxComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!BoundComponent && !BoundComponentName.IsNone())
    {
        TArray<UPrimitiveComponent*> PrimComps;
        GetOwner()->GetComponents<UPrimitiveComponent>(PrimComps);

        for (UPrimitiveComponent* P : PrimComps)
        {
            if (P && P->GetFName() == BoundComponentName)
            {
                BoundComponent = P;
                break;
            }
        }
    }
}

void UBMHurtBoxComponent::ModifyIncomingDamage(FBMDamageInfo& InOutInfo) const
{
    // 1) 部位倍率（头部/弱点等）
    InOutInfo.DamageValue *= DamageMultiplier;

    // 2) 元素弱抗
    if (WeaknessTypes.Contains(InOutInfo.ElementType))
    {
        InOutInfo.DamageValue *= 1.25f;
    }
    if (ResistanceTypes.Contains(InOutInfo.ElementType))
    {
        InOutInfo.DamageValue *= 0.75f;
    }

    // 3) 可选：根据倍率/伤害量推荐受击反馈（示例：倍率高则更重）
    if (InOutInfo.HitReaction == EBMHitReaction::None)
    {
        InOutInfo.HitReaction = (DamageMultiplier >= 1.5f) ? EBMHitReaction::Heavy : EBMHitReaction::Light;
    }
}
