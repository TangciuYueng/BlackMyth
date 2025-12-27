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

    bInFallLoop = false;
    bDidPlayJumpStart = false;

    PC->GetWorldTimerManager().ClearTimer(JumpToFallHandle);

    UCharacterMovementComponent* Move = PC->GetCharacterMovement();
    if (!Move)
    {
        return;
    }

    // 判断这次进入空中是不是玩家主动起跳
    const bool bWantsJumpStart = PC->ConsumePendingJump() && !Move->IsFalling() && PC->CanJump();

    if (bWantsJumpStart)
    {
        bDidPlayJumpStart = true;
        PC->Jump();

        const float JumpStartDuration = PC->PlayJumpStartOnce(1.0f);

        // Jump 动画播完后如果还在空中，则切 FallLoop
        if (JumpStartDuration > 0.f)
        {
            FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
                {
                    StartFallLoopIfStillInAir();
                });
            PC->GetWorldTimerManager().SetTimer(JumpToFallHandle, D, JumpStartDuration, false);
        }
        else
        {
            StartFallLoopIfStillInAir();
        }
    }
    else
    {
        // 非起跳导致的下落直接 FallLoop
        EnterFallLoop();
    }
}

void UBMPlayerState_Jump::OnExit(float DeltaTime)
{
    (void)DeltaTime;

    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->GetWorldTimerManager().ClearTimer(JumpToFallHandle);

    PC->StopJumping();

    bInFallLoop = false;
    bDidPlayJumpStart = false;
}

void UBMPlayerState_Jump::OnUpdate(float DeltaTime)
{
    (void)DeltaTime;

    // 回到地面状态
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
        return;
    }
    // 掉落进入 Jump 状态
    if (!bDidPlayJumpStart && !bInFallLoop)
    {
        EnterFallLoop();
    }
}

void UBMPlayerState_Jump::StartFallLoopIfStillInAir()
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    UCharacterMovementComponent* Move = PC->GetCharacterMovement();
    if (!Move) return;

    if (Move->IsFalling())
    {
        EnterFallLoop();
    }
}

void UBMPlayerState_Jump::EnterFallLoop()
{
    if (bInFallLoop) return;

    if (ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext()))
    {
        PC->PlayFallLoop();
        bInFallLoop = true;
    }
}
