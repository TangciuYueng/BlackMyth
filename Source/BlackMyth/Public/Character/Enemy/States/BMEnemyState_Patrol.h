#pragma once
#include "Character/Components/BMCharacterState.h"
#include "BMEnemyState_Patrol.generated.h"

UCLASS()
class BLACKMYTH_API UBMEnemyState_Patrol : public UBMCharacterState
{
    GENERATED_BODY()
public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnUpdate(float DeltaTime) override;

private:
    float RepathAccum = 0.f;
    FVector PatrolDest = FVector::ZeroVector;
};