#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Idle.generated.h"

UCLASS()
class BLACKMYTH_API UBMPlayerState_Idle : public UBMCharacterState
{
    GENERATED_BODY()

public:
    virtual void OnUpdate(float DeltaTime) override;
    virtual void OnEnter(float DeltaTime) override;
};