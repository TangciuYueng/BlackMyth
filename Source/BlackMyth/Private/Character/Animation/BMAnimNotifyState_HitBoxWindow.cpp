#include "Character/Animation/BMAnimNotifyState_HitBoxWindow.h"
#include "Character/BMCharacterBase.h"
#include "Character/Components/BMHitBoxComponent.h"
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"



DEFINE_LOG_CATEGORY(LogBMHitBoxWindow);

/*
 * @brief Constructor of the UBMAnimNotifyState_HitBoxWindow class
 */
UBMAnimNotifyState_HitBoxWindow::UBMAnimNotifyState_HitBoxWindow()
{
}

/*
 * @brief Notify begin
 * @param MeshComp The mesh component
 * @param Animation The animation
 * @param TotalDuration The total duration
 * @param EventReference The event reference
 */
void UBMAnimNotifyState_HitBoxWindow::NotifyBegin(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!MeshComp) return;

    ABMCharacterBase* OwnerChar = Cast<ABMCharacterBase>(MeshComp->GetOwner());
    if (!OwnerChar)
    {
        return;
    }

    UBMHitBoxComponent* HB = OwnerChar->GetHitBox();
    if (!HB)
    {
        UE_LOG(LogBMHitBoxWindow, Warning, TEXT("[%s] NotifyBegin: No HitBoxComponent. (Window=%s)"),
            *OwnerChar->GetName(), *WindowId.ToString());
        return;
    }

    TArray<FName> Names;
    FBMHitBoxActivationParams Params;
    if (!OwnerChar->ResolveHitBoxWindow(WindowId, Names, Params) || Names.Num() == 0)
    {
        UE_LOG(LogBMHitBoxWindow, Warning, TEXT("[%s] NotifyBegin: No valid HitBox window context. (Window=%s)"),
            *OwnerChar->GetName(), *WindowId.ToString());
        return;
    }

    HB->ActivateHitBoxesByNames(Names, Params);
}

/*
 * @brief Notify end
 * @param MeshComp The mesh component
 * @param Animation The animation
 * @param EventReference The event reference
 */
void UBMAnimNotifyState_HitBoxWindow::NotifyEnd(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    ABMCharacterBase* OwnerChar = Cast<ABMCharacterBase>(MeshComp->GetOwner());
    if (!OwnerChar) return;

    if (UBMHitBoxComponent* HB = OwnerChar->GetHitBox())
    {
        // ������������
        HB->DeactivateAllHitBoxes();
    }
}
