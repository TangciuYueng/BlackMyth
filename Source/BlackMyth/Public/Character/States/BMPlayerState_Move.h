#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Move.generated.h"

UCLASS()
class BLACKMYTH_API UBMPlayerState_Move : public UBMCharacterState
{
    GENERATED_BODY()

public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnUpdate(float DeltaTime) override;

private:
    float StopSpeedThreshold = 3.f; // »ù±¾¾²Ö¹ãÐÖµ
};
