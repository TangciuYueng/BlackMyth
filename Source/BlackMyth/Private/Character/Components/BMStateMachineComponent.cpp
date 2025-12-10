#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMCharacterState.h"
#include "Core/BMTypes.h"

UBMStateMachineComponent::UBMStateMachineComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 由 ABMCharacterBase Tick 统一驱动
}

void UBMStateMachineComponent::RegisterState(FName Name, UBMCharacterState* State)
{
    if (Name.IsNone() || !State) return;
    States.Add(Name, State);
}

void UBMStateMachineComponent::ChangeState(UBMCharacterState* NewState)
{
    if (NewState == Current) return;

    if (Current)
    {
        Current->OnExit(0.f);
    }
    Current = NewState;

    // 反查名字
    CurrentStateName = NAME_None;
    for (const auto& It : States)
    {
        if (It.Value == Current)
        {
            CurrentStateName = It.Key;
            break;
        }
    }

    if (Current)
    {
        Current->OnEnter(0.f);
    }
}

bool UBMStateMachineComponent::ChangeStateByName(FName Name)
{
    TObjectPtr<UBMCharacterState>* Found = States.Find(Name);
    if (!Found || !(*Found)) return false;

    if (Current && !Current->CanTransitionTo(Name))
    {
        return false;
    }

    ChangeState(*Found);
    return true;
}

void UBMStateMachineComponent::TickState(float DeltaSeconds)
{
    if (Current)
    {
        Current->OnUpdate(DeltaSeconds);
    }
}
