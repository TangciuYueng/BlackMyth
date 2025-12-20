#include "Character/States/BMPlayerState_Hit.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/BMTypes.h"

void UBMPlayerState_Hit::OnEnter(float)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->GetWorldTimerManager().ClearTimer(TimerHandle);

    bFinished = false;

    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(true);
    }

    const float Duration = PC->PlayHitOnce(PC->GetLastDamageInfo());
    if (Duration <= 0.f)
    {
        bFinished = true;
        if (PC->GetFSM())
        {
            PC->GetFSM()->ChangeStateByName(PC->HasMoveIntent() ? BMStateNames::Move : BMStateNames::Idle);
        }
        return;
    }

    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            ABMPlayerCharacter* PC2 = Cast<ABMPlayerCharacter>(GetContext());
            if (!PC2) return;

            bFinished = true;

            if (UBMCombatComponent* Combat2 = PC2->GetCombat())
            {
                Combat2->SetActionLock(false);
            }

            if (UCharacterMovementComponent* Move = PC2->GetCharacterMovement(); Move && Move->IsFalling())
            {
                if (PC2->GetFSM()) PC2->GetFSM()->ChangeStateByName(BMStateNames::Jump);
                return;
            }

            if (PC2->GetFSM())
            {
                PC2->GetFSM()->ChangeStateByName(PC2->HasMoveIntent() ? BMStateNames::Move : BMStateNames::Idle);
            }
        });

    PC->GetWorldTimerManager().SetTimer(TimerHandle, D, Duration, false);
}

void UBMPlayerState_Hit::OnExit(float)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->GetWorldTimerManager().ClearTimer(TimerHandle);

    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(false);
    }

    bFinished = false;
}

bool UBMPlayerState_Hit::CanTransitionTo(FName StateName) const
{
    if (StateName == BMStateNames::Death) return true;
    return bFinished;
}
