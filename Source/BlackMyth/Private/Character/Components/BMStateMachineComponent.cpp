#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMCharacterState.h"
#include "Core/BMTypes.h"

/*
 * @brief Constructor of the UBMStateMachineComponent class
 */
UBMStateMachineComponent::UBMStateMachineComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // �� ABMCharacterBase Tick ͳһ����
}

/*
 * @brief Register state, it registers the state
 * @param Name The name of the state
 * @param State The state
 */
void UBMStateMachineComponent::RegisterState(FName Name, UBMCharacterState* State)
{
    if (Name.IsNone() || !State) return;
    States.Add(Name, State);
}

/*
 * @brief Change state, it changes the state
 * @param NewState The new state
 */
void UBMStateMachineComponent::ChangeState(UBMCharacterState* NewState)
{
    if (NewState == Current) return;

    if (Current)
    {
        Current->OnExit(0.f);
    }
    Current = NewState;

    // ��������
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

/*
 * @brief Change state by name, it changes the state by name
 * @param Name The name of the state
 * @return True if the state is changed, false otherwise
 */
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

/*
 * @brief Tick state, it ticks the state
 * @param DeltaSeconds The delta seconds
 */
void UBMStateMachineComponent::TickState(float DeltaSeconds)
{
    if (Current)
    {
        Current->OnUpdate(DeltaSeconds);
    }
}
