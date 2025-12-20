#pragma once
#include "Character/Components/BMCharacterState.h"
#include "BMEnemyState_Chase.generated.h"

UCLASS()
class BLACKMYTH_API UBMEnemyState_Chase : public UBMCharacterState
{
    GENERATED_BODY()
public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnUpdate(float DeltaTime) override;
};