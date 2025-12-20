#pragma once
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Hit.generated.h"

UCLASS()
class BLACKMYTH_API UBMPlayerState_Hit : public UBMCharacterState
{
    GENERATED_BODY()
public:
    virtual void OnEnter(float) override;
    virtual void OnExit(float) override;
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    FTimerHandle TimerHandle;
    bool bFinished = false;
};