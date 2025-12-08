#include "Character/States/BMPlayerState_Attack.h"

#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Core/BMTypes.h"

void UBMPlayerState_Attack::OnEnter(float DeltaTime)
{
    (void)DeltaTime;

    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    // 锁动作：攻击中不允许再次触发攻击/跳跃等（你后续可做更细粒度）
    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(true);
    }

    // 攻击时先停一下移动（是否允许移动你可再调）
    if (UCharacterMovementComponent* Move = PC->GetCharacterMovement())
    {
        Move->StopMovementImmediately();
    }

    bFinished = false;

    const float Duration = PC->PlayLightAttackOnce(1.0f);
    if (Duration <= 0.f)
    {
        FinishAttack(false);
        return;
    }

    // 用 Timer 代替 Montage 回调（纯C++、不需要 AnimGraph Slot）
    PC->GetWorldTimerManager().SetTimer(TimerHandle, [this]()
        {
            FinishAttack(false);
        }, Duration, false);
}

void UBMPlayerState_Attack::OnExit(float DeltaTime)
{
    (void)DeltaTime;

    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;


    PC->GetWorldTimerManager().ClearTimer(TimerHandle);

    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(false);
    }

    bFinished = false;
}

bool UBMPlayerState_Attack::CanTransitionTo(FName StateName) const
{
    // 攻击过程中默认不允许切走（除非你以后加 Death/Hit 强制切）
    // 这里先简单：只有完成后才允许切换
    if (!bFinished)
    {
        // 允许死亡强制切（后面你加 Death 状态时就能直接用）
        if (StateName == BMStateNames::Death)
        {
            return true;
        }
        return false;
    }
    return true;
}

void UBMPlayerState_Attack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != PlayingMontage) return;
    FinishAttack(bInterrupted);
}

void UBMPlayerState_Attack::FinishAttack(bool bInterrupted)
{
    (void)bInterrupted;

    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    bFinished = true;

    // 退出时解锁
    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(false);
    }

    // 回到合适状态：空中 -> Jump，地面 -> Move/Idle
    if (UCharacterMovementComponent* Move = PC->GetCharacterMovement())
    {
        if (Move->IsFalling())
        {
            if (PC->GetFSM()) PC->GetFSM()->ChangeStateByName(BMStateNames::Jump);
            return;
        }
    }

    if (PC->GetFSM())
    {
        PC->GetFSM()->ChangeStateByName(PC->HasMoveIntent() ? BMStateNames::Move : BMStateNames::Idle);
    }
}
