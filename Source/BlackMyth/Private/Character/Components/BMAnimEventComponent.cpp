#include "Character/Components/BMAnimEventComponent.h"
#include "Character/BMCharacterBase.h"
#include "Character/Components/BMHitBoxComponent.h"

UBMAnimEventComponent::UBMAnimEventComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

ABMCharacterBase* UBMAnimEventComponent::GetOwnerCharacter() const
{
    return Cast<ABMCharacterBase>(GetOwner());
}

void UBMAnimEventComponent::Anim_OpenHitBox(EBMHitBoxType Type)
{
    if (ABMCharacterBase* C = GetOwnerCharacter())
    {
        if (UBMHitBoxComponent* HB = C->GetHitBox())
        {
            HB->ActivateHitBox(Type);
        }
    }
}

void UBMAnimEventComponent::Anim_CloseHitBox()
{
    if (ABMCharacterBase* C = GetOwnerCharacter())
    {
        if (UBMHitBoxComponent* HB = C->GetHitBox())
        {
            HB->DeactivateHitBox();
        }
    }
}

void UBMAnimEventComponent::Anim_ResetHitList()
{
    if (ABMCharacterBase* C = GetOwnerCharacter())
    {
        if (UBMHitBoxComponent* HB = C->GetHitBox())
        {
            HB->ResetHitList();
        }
    }
}

void UBMAnimEventComponent::Anim_PublishEvent(FName EventTag)
{
    (void)EventTag;
    // 你后面接 EventBusSubsystem 时在这里 publish(FBMEventData)
}

void UBMAnimEventComponent::Anim_SpawnEffect(FName EffectName)
{
    (void)EffectName;
}
