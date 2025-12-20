#pragma once
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Death.generated.h"

UCLASS()
class BLACKMYTH_API UBMPlayerState_Death : public UBMCharacterState
{
    GENERATED_BODY()
public:
    virtual void OnEnter(float) override;
    virtual bool CanTransitionTo(FName) const override { return false; }
};
