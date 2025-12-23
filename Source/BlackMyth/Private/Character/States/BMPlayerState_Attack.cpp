#include "Character/States/BMPlayerState_Attack.h"

#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/BMTypes.h"

void UBMPlayerState_Attack::OnEnter(float)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->GetWorldTimerManager().ClearTimer(TimerStepEnd);
    PC->GetWorldTimerManager().ClearTimer(TimerWindowOpen);
    PC->GetWorldTimerManager().ClearTimer(TimerWindowClose);
    PC->GetWorldTimerManager().ClearTimer(TimerPollInput);
    PC->GetWorldTimerManager().ClearTimer(TimerRecoverEnd);

    bFinished = false;
	bRecover = false;
    bIsCombo = false;
    ComboIndex = -1;
    bLinkWindowOpen = false;
    bQueuedNext = false;

    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(true); // 仍锁住移动/其它动作
    }

    // 空中禁止攻击
    if (UCharacterMovementComponent* Move = PC->GetCharacterMovement(); Move && Move->IsFalling())
    {
        FinishAttack(true);
        return;
    }

    // 惯性参数
    ApplyAttackInertiaSettings(PC->GetCharacterMovement());

    // 从队列取第一个动作
    EBMCombatAction Action = EBMCombatAction::None;
    if (!PC->ConsumeNextQueuedAction(Action))
    {
        FinishAttack(true);
        return;
    }

    if (Action == EBMCombatAction::NormalAttack)
    {
        StartComboStep(0);
        return;
    }

    if (BMCombatUtils::IsSkillAction(Action))
    {
        FBMPlayerAttackSpec Spec;
        float StaminaCost = 0.f;
        if (!PC->SelectSkillSpec(Action, Spec, StaminaCost))
        {
            FinishAttack(true);
            return;
        }

        // 冷却
        if (UBMCombatComponent* Combat = PC->GetCombat())
        {
            if (!Combat->IsCooldownReady(Spec.Id))
            {
                FinishAttack(true);
                return;
            }
        }

        if (UBMStatsComponent* Stats = PC->GetStats())
        {
            if (!Stats->TryConsumeStamina(StaminaCost))
            {
                FinishAttack(true);
                if (UBMStateMachineComponent* M = PC->GetFSM())
                {
                    M->ChangeStateByName(BMStateNames::Idle);
                }
                return;
            }
        }

        StartSkill(Spec);
        return;
    }

    FinishAttack(true);
}

void UBMPlayerState_Attack::OnExit(float)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->GetWorldTimerManager().ClearTimer(TimerStepEnd);
    PC->GetWorldTimerManager().ClearTimer(TimerWindowOpen);
    PC->GetWorldTimerManager().ClearTimer(TimerWindowClose);
    PC->GetWorldTimerManager().ClearTimer(TimerPollInput);
    PC->GetWorldTimerManager().ClearTimer(TimerRecoverEnd);

    PC->ClearActiveAttackContext();

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
    if (StateName == BMStateNames::Dodge) return bRecover;
    return bFinished;
}

// ---------------- combo ----------------

void UBMPlayerState_Attack::StartComboStep(int32 StepIndex)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    FBMPlayerComboStep Step;
    if (!PC->GetComboStep(StepIndex, Step))
    {
        // 没有该段，直接收招
        OnRecoverFinished();
        return;
    }

    bIsCombo = true;
    ComboIndex = StepIndex;

    bLinkWindowOpen = false;
    bQueuedNext = false;

    // 设置攻击上下文
    PC->SetActiveAttackContext(
        Step.HitBoxNames,
        Step.HitBoxParams,
        Step.bUninterruptible,
        Step.InterruptChance,
        Step.InterruptChanceOnHeavyHit
    );

    // 播放本段动画
    const float Duration = PC->PlayNormalAttackOnce(Step.Anim, Step.PlayRate, Step.StartTime, Step.MaxPlayTime);
    if (Duration <= 0.f)
    {
        OnRecoverFinished();
        return;
    }

    // 最后 LinkWindowSeconds 秒内允许连段
    float OpenT = FMath::Max(0.f, Duration - Step.LinkWindowSeconds);
    float CloseT = FMath::Max(OpenT, Duration - Step.LinkWindowEndOffset);

    // 计时器：开/关窗口
    PC->GetWorldTimerManager().SetTimer(
        TimerWindowOpen, this, &UBMPlayerState_Attack::OpenLinkWindow, OpenT, false);

    PC->GetWorldTimerManager().SetTimer(
        TimerWindowClose, this, &UBMPlayerState_Attack::CloseLinkWindow, CloseT, false);

    // 轮询输入
    PC->GetWorldTimerManager().SetTimer(
        TimerPollInput, this, &UBMPlayerState_Attack::PollComboInput, 1.f / 60.f, true);

    // 本段结束
    PC->GetWorldTimerManager().SetTimer(
        TimerStepEnd, this, &UBMPlayerState_Attack::OnStepFinished, Duration, false);
}

void UBMPlayerState_Attack::OpenLinkWindow()
{
    bLinkWindowOpen = true;
}

void UBMPlayerState_Attack::CloseLinkWindow()
{
    bLinkWindowOpen = false;
}

void UBMPlayerState_Attack::PollComboInput()
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    // 必须消队列里的 NormalAttack
    const bool bPressed = PC->ConsumeOneQueuedNormalAttack();
    if (!bPressed) return;

    // 只有窗口开时才算有效
    if (bLinkWindowOpen)
    {
        bQueuedNext = true;
    }
    // else：窗口外按键作废
}

void UBMPlayerState_Attack::OnStepFinished()
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->GetWorldTimerManager().ClearTimer(TimerPollInput);

    const int32 MaxIdx = PC->GetComboStepCount() - 1;
    const bool bHasNext = (ComboIndex >= 0 && ComboIndex < MaxIdx);

    if (bQueuedNext && bHasNext)
    {
        StartComboStep(ComboIndex + 1);
        return;
    }

    // 没有下一段 / 没接上：收招回 Idle
    StartRecoverForStep(ComboIndex);
}

void UBMPlayerState_Attack::StartRecoverForStep(int32 FromStepIndex)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    // Recover 不应产生命中
    PC->ClearActiveAttackContext();

    PC->GetWorldTimerManager().ClearTimer(TimerWindowOpen);
    PC->GetWorldTimerManager().ClearTimer(TimerWindowClose);
    PC->GetWorldTimerManager().ClearTimer(TimerPollInput);

    FBMPlayerComboStep Step;
    if (!PC->GetComboStep(FromStepIndex, Step))
    {
        OnRecoverFinished();
        return;
    }

    const float Dur = PC->PlayComboRecoverOnce(Step);
    if (Dur <= 0.f)
    {
        OnRecoverFinished();
        return;
    }
	bRecover = true;
    PC->GetWorldTimerManager().SetTimer(
        TimerRecoverEnd, this, &UBMPlayerState_Attack::OnRecoverFinished, Dur, false);
}


void UBMPlayerState_Attack::OnRecoverFinished()
{
    FinishAttack(false);

    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    // 回到 Idle
    if (PC->GetFSM())
    {
        PC->GetFSM()->ChangeStateByName(BMStateNames::Idle);
    }
}


void UBMPlayerState_Attack::StartSkill(const FBMPlayerAttackSpec& Spec)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    bIsCombo = false;

    // active context
    PC->SetActiveAttackContext(
        Spec.HitBoxNames,
        Spec.HitBoxParams,
        Spec.bUninterruptible,
        Spec.InterruptChance,
        Spec.InterruptChanceOnHeavyHit
    );

    const float Duration = PC->PlayAttackOnce(Spec);
    if (Duration <= 0.f)
    {
        FinishAttack(true);
        return;
    }

    // 提交冷却（技能）
    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->CommitCooldown(Spec.Id, Spec.Cooldown);
    }

    PC->GetWorldTimerManager().SetTimer(
        TimerStepEnd, this, &UBMPlayerState_Attack::OnRecoverFinished, Duration, false);
}

// ---------------- common ----------------

void UBMPlayerState_Attack::FinishAttack(bool)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    bFinished = true;

    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(false);
    }
}

void UBMPlayerState_Attack::ApplyAttackInertiaSettings(UCharacterMovementComponent* Move)
{
    if (!Move) return;

    SavedGroundFriction = Move->GroundFriction;
    SavedBrakingDecelWalking = Move->BrakingDecelerationWalking;
    SavedBrakingFrictionFactor = Move->BrakingFrictionFactor;
    bSavedUseSeparateBrakingFriction = Move->bUseSeparateBrakingFriction;
    SavedBrakingFriction = Move->BrakingFriction;

    Move->GroundFriction = 2.0f;
    Move->BrakingDecelerationWalking = 350.f;
    Move->BrakingFrictionFactor = 0.6f;
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
