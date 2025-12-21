#include "Character/Enemy/States/BMEnemyBossState_PhaseChange.h"

#include "Character/Enemy/BMEnemyBoss.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/BMTypes.h"

void UBMEnemyBossState_PhaseChange::OnEnter(float)
{
    ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(GetContext());
    if (!Boss) return;

    bFinished = false;

    Boss->GetWorldTimerManager().ClearTimer(StepTimer);

    // 1) 锁动作，防止其它状态抢占
    if (UBMCombatComponent* C = Boss->GetCombat())
    {
        C->SetActionLock(true);
    }

    // 2) 停止移动 + 禁止移动
    Boss->RequestStopMovement();
    if (UCharacterMovementComponent* Move = Boss->GetCharacterMovement())
    {
        Move->StopMovementImmediately();
        Move->DisableMovement();
    }

    // 3) 过渡期间无敌
    Boss->SetPhaseTransition(true);

    // 4) 先播死亡动画
    const float DeathDur = Boss->PlayDeathOnce();
    if (DeathDur <= 0.f)
    {
        StartHoldAfterDeath();
        return;
    }

    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            StartHoldAfterDeath();
        });

    Boss->GetWorldTimerManager().SetTimer(StepTimer, D, DeathDur, false);
}

void UBMEnemyBossState_PhaseChange::StartHoldAfterDeath()
{
    ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(GetContext());
    if (!Boss) return;

    const float Hold = FMath::Max(0.f, Boss->GetPhase2DeathHoldSeconds());
    if (Hold <= 0.f)
    {
        StartDeathReverse();
        return;
    }

    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            StartDeathReverse();
        });
    Boss->GetWorldTimerManager().SetTimer(StepTimer, D, Hold, false);
}

void UBMEnemyBossState_PhaseChange::StartDeathReverse()
{
    ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(GetContext());
    if (!Boss) return;

    const float Dur = Boss->PlayDeathReverseOnce(
        Boss->GetPhase2DeathReversePlayRate(),
        Boss->GetPhase2DeathReverseMaxTime()
    );

    if (Dur <= 0.f)
    {
        StartEnergize();
        return;
    }

    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            StartEnergize();
        });
    Boss->GetWorldTimerManager().SetTimer(StepTimer, D, Dur, false);
}

void UBMEnemyBossState_PhaseChange::OnExit(float)
{
    ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(GetContext());
    if (!Boss) return;

    Boss->GetWorldTimerManager().ClearTimer(StepTimer);

    // 解除锁与无敌在 FinishPhaseChange 里统一做
    bFinished = false;
}

bool UBMEnemyBossState_PhaseChange::CanTransitionTo(FName StateName) const
{
    // 过渡期间不可被打断
    if (StateName == BMEnemyStateNames::Death) return true;
    return bFinished;
}

void UBMEnemyBossState_PhaseChange::StartEnergize()
{
    ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(GetContext());
    if (!Boss) return;

    const float EnergizeDur = Boss->PlayEnergizeOnce();
    if (EnergizeDur <= 0.f)
    {
        FinishPhaseChange();
        return;
    }

    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            FinishPhaseChange();
        });

    Boss->GetWorldTimerManager().SetTimer(StepTimer, D, EnergizeDur, false);
}

void UBMEnemyBossState_PhaseChange::FinishPhaseChange()
{
    ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(GetContext());
    if (!Boss) return;

    // 进入二阶段（复活、加血、加伤害、加招式）
    Boss->EnterPhase2();

    // 恢复移动
    if (UCharacterMovementComponent* Move = Boss->GetCharacterMovement())
    {
        Move->SetMovementMode(MOVE_Walking);
    }

    // 解除无敌与动作锁
    Boss->SetPhaseTransition(false);
    if (UBMCombatComponent* C = Boss->GetCombat())
    {
        C->SetActionLock(false);
    }

    bFinished = true;

    // 回到 Idle
    if (UBMStateMachineComponent* FSM = Boss->GetFSM())
    {
        FSM->ChangeStateByName(BMEnemyStateNames::Idle);
    }

    
}
