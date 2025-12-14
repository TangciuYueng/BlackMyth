#include "Character/States/BMPlayerState_Idle.h"

#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/BMTypes.h"

void UBMPlayerState_Idle::OnUpdate(float DeltaTime)
{
    (void)DeltaTime;

    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    if (UCharacterMovementComponent* Move = PC->GetCharacterMovement())
    {
        if (Move->IsFalling())
        {
            if (PC->GetFSM()) PC->GetFSM()->ChangeStateByName(BMStateNames::Jump);
            return;
        }
    }

    if (PC->HasMoveIntent())
    {
        if (PC->GetFSM()) PC->GetFSM()->ChangeStateByName(BMStateNames::Move);
    }
}

void UBMPlayerState_Idle::OnEnter(float DeltaTime)
{
    (void)DeltaTime;
    if (ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext()))
    {
        PC->PlayIdleLoop();
    }
}