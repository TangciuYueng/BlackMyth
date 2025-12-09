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

    UCharacterMovementComponent* Move = PC->GetCharacterMovement();
    // 锁动作：攻击中不允许再次触发攻击/跳跃等
    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(true);
    }
    ApplyAttackInertiaSettings(Move);

    bFinished = false;

    const float Duration = PC->PlayLightAttackOnce(1.0f);
    if (Duration <= 0.f)
    {
        FinishAttack(false);
        return;
    }

    // 用 Timer 代替 Montage 回调
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
    // 退出攻击恢复移动参数
    RestoreMovementSettings(PC->GetCharacterMovement());

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

void UBMPlayerState_Attack::ApplyAttackInertiaSettings(UCharacterMovementComponent* Move)
{
    if (!Move) return;

    // 先保存
    SavedGroundFriction = Move->GroundFriction;
    SavedBrakingDecelWalking = Move->BrakingDecelerationWalking;
    SavedBrakingFrictionFactor = Move->BrakingFrictionFactor;
    bSavedUseSeparateBrakingFriction = Move->bUseSeparateBrakingFriction;
    SavedBrakingFriction = Move->BrakingFriction;

    // 再设置：让“无输入时的减速”更平滑一点，从而产生惯性
    Move->GroundFriction = 2.0f;              // 默认通常较大，降低一点 -> 更滑
    Move->BrakingDecelerationWalking = 350.f; // 默认可能很大，降低 -> 不会瞬停
    Move->BrakingFrictionFactor = 0.6f;       // <1 更滑，>1 更刹

    //单独的刹车摩擦，手感更可控
    Move->bUseSeparateBrakingFriction = true;
    Move->BrakingFriction = 2.0f;
}

void UBMPlayerState_Attack::RestoreMovementSettings(UCharacterMovementComponent* Move)
{
    if (!Move) return;

    Move->GroundFriction = SavedGroundFriction;
    Move->BrakingDecelerationWalking = SavedBrakingDecelWalking;
    Move->BrakingFrictionFactor = SavedBrakingFrictionFactor;
    Move->bUseSeparateBrakingFriction = bSavedUseSeparateBrakingFriction;
    Move->BrakingFriction = SavedBrakingFriction;
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
