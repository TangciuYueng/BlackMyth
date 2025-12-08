#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMStateMachineComponent.generated.h"


class UBMCharacterState;

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMStateMachineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMStateMachineComponent();

    void RegisterState(FName Name, UBMCharacterState* State);
    void ChangeState(UBMCharacterState* NewState);
    bool ChangeStateByName(FName Name);
    bool ChangeStateById(EBMCharacterStateId Id)
    {
        return ChangeStateByName(BMStateNames::ToName(Id));
    }
    void TickState(float DeltaSeconds);

    FName GetCurrentStateName() const { return CurrentStateName; }
    UBMCharacterState* GetCurrentState() const { return Current; }



    
private:
    UPROPERTY()
    TObjectPtr<UBMCharacterState> Current = nullptr;

    UPROPERTY()
    TMap<FName, TObjectPtr<UBMCharacterState>> States;

    FName CurrentStateName = NAME_None;
};
