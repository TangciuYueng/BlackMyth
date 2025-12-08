#include "Character/States/BMPlayerState_Jump.h"

#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/BMTypes.h"

void UBMPlayerState_Jump::OnEnter(float DeltaTime)
{
    (void)DeltaTime;

    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    // 真正起跳
    PC->PlayJumpLoop();
    PC->Jump();
}

void UBMPlayerState_Jump::OnExit(float DeltaTime)
{
    (void)DeltaTime;

    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->StopJumping();
}

void UBMPlayerState_Jump::OnUpdate(float DeltaTime)
{
    (void)DeltaTime;

    // 兜底：如果某些情况下没走 Landed 回调，也可以靠这里回到地面状态
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    UCharacterMovementComponent* Move = PC->GetCharacterMovement();
    if (!Move) return;

    if (!Move->IsFalling())
    {
        if (PC->GetFSM())
        {
            PC->GetFSM()->ChangeStateByName(PC->HasMoveIntent() ? BMStateNames::Move : BMStateNames::Idle);
        }
    }
}
