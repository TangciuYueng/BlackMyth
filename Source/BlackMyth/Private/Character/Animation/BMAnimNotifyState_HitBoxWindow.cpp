#include "Character/Animation/BMAnimNotifyState_HitBoxWindow.h"

#include "Character/BMCharacterBase.h"
#include "Character/Components/BMHitBoxComponent.h"

void UBMAnimNotifyState_HitBoxWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    (void)Animation;
    (void)TotalDuration;

    if (!MeshComp) return;

    ABMCharacterBase* OwnerChar = Cast<ABMCharacterBase>(MeshComp->GetOwner());
    if (!OwnerChar) return;

    if (UBMHitBoxComponent* HB = OwnerChar->GetHitBox())
    {
        if (bResetHitListOnBegin)
        {
            HB->ResetHitList();
        }
        HB->ActivateHitBox(HitBoxType);
    }
}

void UBMAnimNotifyState_HitBoxWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    (void)Animation;

    if (!MeshComp) return;

    ABMCharacterBase* OwnerChar = Cast<ABMCharacterBase>(MeshComp->GetOwner());
    if (!OwnerChar) return;

    if (UBMHitBoxComponent* HB = OwnerChar->GetHitBox())
    {
        HB->DeactivateHitBox();
    }
}
