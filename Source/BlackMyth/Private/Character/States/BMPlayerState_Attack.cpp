#include "Character/States/BMPlayerState_Attack.h"

#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Core/BMTypes.h"

void UBMPlayerState_Attack::OnEnter(float)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->GetWorldTimerManager().ClearTimer(TimerHandle);

    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(true);
    }

    // 空中禁止攻击
    if (UCharacterMovementComponent* Move = PC->GetCharacterMovement(); Move && Move->IsFalling())
    {
        FinishAttack(true);
        return;
    }

    // 取 PendingAction
    EBMCombatAction Action = EBMCombatAction::LightAttack;
    PC->ConsumePendingAction(Action);

    // 选招式 Spec
    FBMPlayerAttackSpec Spec;
    if (!PC->SelectAttackSpec(Action, Spec))
    {
        FinishAttack(true);
        return;
    }

    // 锁定当前招式，供“被打断判定”使用
    PC->SetActiveAttackSpec(Spec);

    // 惯性参数
    ApplyAttackInertiaSettings(PC->GetCharacterMovement());

    bFinished = false;

    const float Duration = PC->PlayAttackOnce(Spec);
    if (Duration <= 0.f)
    {
        FinishAttack(false);
        return;
    }

    // 提交该招式冷却（以 Spec.Id 为 Key）
    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->CommitCooldown(Spec.Id, Spec.Cooldown);
    }

    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            FinishAttack(false);
        });
    PC->GetWorldTimerManager().SetTimer(TimerHandle, D, Duration, false);
}

void UBMPlayerState_Attack::OnExit(float)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->GetWorldTimerManager().ClearTimer(TimerHandle);

    PC->ClearActiveAttackSpec();

    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(false);
    }

    RestoreMovementSettings(PC->GetCharacterMovement());
    bFinished = false;
}

bool UBMPlayerState_Attack::CanTransitionTo(FName StateName) const
{
    if (StateName == BMStateNames::Death) return true;
    if (StateName == BMStateNames::Hit)   return true; 
    return bFinished;
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

    //单独的刹车摩擦
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
