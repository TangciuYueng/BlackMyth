#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Jump.generated.h"

UCLASS()
class BLACKMYTH_API UBMPlayerState_Jump : public UBMCharacterState
{
    GENERATED_BODY()

public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnExit(float DeltaTime) override;
    virtual void OnUpdate(float DeltaTime) override;
};
