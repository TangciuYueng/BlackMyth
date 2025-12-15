#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Core/BMTypes.h"
#include "BMAnimNotifyState_HitBoxWindow.generated.h"

UCLASS()
class BLACKMYTH_API UBMAnimNotifyState_HitBoxWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "BM|HitBox")
    EBMHitBoxType HitBoxType = EBMHitBoxType::LightAttack;

    UPROPERTY(EditAnywhere, Category = "BM|HitBox")
    bool bResetHitListOnBegin = true;

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
