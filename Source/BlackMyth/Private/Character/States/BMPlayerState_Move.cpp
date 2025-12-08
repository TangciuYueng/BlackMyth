#include "Character/States/BMPlayerState_Move.h"

#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMStatsComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/BMTypes.h"

void UBMPlayerState_Move::OnEnter(float DeltaTime)
{
    (void)DeltaTime;

    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    // 进入 Move 时同步移动速度（从 Stats）
    if (UBMStatsComponent* Stats = PC->GetStats())
    {
        if (UCharacterMovementComponent* Move = PC->GetCharacterMovement())
        {
            Move->MaxWalkSpeed = Stats->GetStatBlock().MoveSpeed;
        }
    }

    PC->PlayMoveLoop();
}

void UBMPlayerState_Move::OnUpdate(float DeltaTime)
{
    (void)DeltaTime;

    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    UCharacterMovementComponent* Move = PC->GetCharacterMovement();
    if (!Move) return;

    if (Move->IsFalling())
    {
        if (PC->GetFSM()) PC->GetFSM()->ChangeStateByName(BMStateNames::Jump);
        return;
    }

    const float Speed2D = Move->Velocity.Size2D();
    if (!PC->HasMoveIntent() && Speed2D <= StopSpeedThreshold)
    {
        if (PC->GetFSM()) PC->GetFSM()->ChangeStateByName(BMStateNames::Idle);
    }
}
