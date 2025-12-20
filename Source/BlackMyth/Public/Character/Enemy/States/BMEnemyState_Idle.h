#pragma once
#include "Character/Components/BMCharacterState.h"
#include "BMEnemyState_Idle.generated.h"

UCLASS()
class BLACKMYTH_API UBMEnemyState_Idle : public UBMCharacterState
{
    GENERATED_BODY()
public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnUpdate(float DeltaTime) override;
};
