#include "Character/Enemy/States/BMEnemyBossState_PhaseChange.h"

#include "Character/Enemy/BMEnemyBoss.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/BMTypes.h"

/*
 * @brief On enter, it enters the phase change state
 * @param DeltaTime The delta time
 */
void UBMEnemyBossState_PhaseChange::OnEnter(float)
{
    ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(GetContext());
    if (!Boss) return;

    bFinished = false;

    Boss->GetWorldTimerManager().ClearTimer(StepTimer);

    // ����������ֹ����״̬��ռ
    if (UBMCombatComponent* C = Boss->GetCombat())
    {
        C->SetActionLock(true);
    }

    // ֹͣ�ƶ� + ��ֹ�ƶ�
    Boss->RequestStopMovement();
    if (UCharacterMovementComponent* Move = Boss->GetCharacterMovement())
    {
        Move->StopMovementImmediately();
        Move->DisableMovement();
    }

    // �����ڼ��޵�
    Boss->SetPhaseTransition(true);

    // �Ȳ���������
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

/*
 * @brief Start death reverse, it starts the death reverse, it plays the death reverse animation and starts the energize
 */
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

/*
 * @brief On exit, it exits the phase change state
 * @param DeltaTime The delta time
 */
void UBMEnemyBossState_PhaseChange::OnExit(float)
{
    ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(GetContext());
    if (!Boss) return;

    Boss->GetWorldTimerManager().ClearTimer(StepTimer);

    // ��������޵��� FinishPhaseChange ��ͳһ��
    bFinished = false;
}

/*
 * @brief Can transition to, it checks if the state can transition to the given state
 * @param StateName The name of the state to transition to
 * @return True if the state can transition to the given state, false otherwise
 */
bool UBMEnemyBossState_PhaseChange::CanTransitionTo(FName StateName) const
{
    // �����ڼ䲻�ɱ����
    if (StateName == BMEnemyStateNames::Death) return true;
    return bFinished;
}

/*
 * @brief Start energize, it starts the energize
 */
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

/*
 * @brief Finish phase change, it finishes the phase change
 */
void UBMEnemyBossState_PhaseChange::FinishPhaseChange()
{
    ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(GetContext());
    if (!Boss) return;

    // ������׶�
    Boss->EnterPhase2();

    // �ָ��ƶ�
    if (UCharacterMovementComponent* Move = Boss->GetCharacterMovement())
    {
        Move->SetMovementMode(MOVE_Walking);
    }

    // ����޵��붯����
    Boss->SetPhaseTransition(false);
    if (UBMCombatComponent* C = Boss->GetCombat())
    {
        C->SetActionLock(false);
    }

    bFinished = true;

    // �ص� Idle
    if (UBMStateMachineComponent* FSM = Boss->GetFSM())
    {
        FSM->ChangeStateByName(BMEnemyStateNames::Idle);
    }
}
