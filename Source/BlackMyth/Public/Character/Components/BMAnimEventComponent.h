#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMAnimEventComponent.generated.h"

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMAnimEventComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMAnimEventComponent();

    void Anim_OpenHitBox(EBMHitBoxType Type);
    void Anim_CloseHitBox();
    void Anim_ResetHitList();

    void Anim_PublishEvent(FName EventTag);
    void Anim_SpawnEffect(FName EffectName);

private:
    class ABMCharacterBase* GetOwnerCharacter() const;
};
