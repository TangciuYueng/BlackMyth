#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BMCharacterState.generated.h"

class ABMCharacterBase;

UCLASS(Abstract)
class BLACKMYTH_API UBMCharacterState : public UObject
{
    GENERATED_BODY()

public:
    virtual void Init(ABMCharacterBase* Owner);

    virtual void OnEnter(float DeltaTime) {}
    virtual void OnUpdate(float DeltaTime) {}
    virtual void OnExit(float DeltaTime) {}

    virtual bool CanTransitionTo(FName StateName) const { return true; }

    ABMCharacterBase* GetContext() const { return Context.Get(); }

protected:
    TWeakObjectPtr<ABMCharacterBase> Context;
};
