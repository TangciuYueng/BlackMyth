#include "Character/States/BMPlayerState_Death.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

void UBMPlayerState_Death::OnEnter(float)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(true);
    }

    if (UCharacterMovementComponent* Move = PC->GetCharacterMovement())
    {
        Move->DisableMovement();
    }

    if (UCapsuleComponent* Cap = PC->GetCapsuleComponent())
    {
        Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    PC->PlayDeathOnce();
}
