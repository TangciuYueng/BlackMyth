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

    // 1) 判断这次进入空中是不是“玩家主动起跳”
    const bool bWantsJumpStart = PC->ConsumePendingJump() && !Move->IsFalling() && PC->CanJump();

    if (bWantsJumpStart)
    {
        // 播起跳动画（一次性） + 执行 Jump()
        bDidPlayJumpStart = true;
        PC->Jump();

        const float JumpStartDuration = PC->PlayJumpStartOnce(1.0f);

        // Jump 动画播完后：如果还在空中，则切 FallLoop
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
            // 没配动画也要能跑：立刻按“仍在空中就 FallLoop”
            StartFallLoopIfStillInAir();
        }
    }
    else
    {
        // 2) 非起跳导致的下落（如走出平台边缘）：直接 FallLoop
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
        return;
    }
    // 如果是“走边缘掉落”进入 Jump 状态
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
