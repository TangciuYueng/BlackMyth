#include "Character/Enemy/States/BMEnemyState_Attack.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMHitBoxComponent.h"
#include "Core/BMTypes.h"
#include "GameFramework/CharacterMovementComponent.h"


/*
 * @brief On enter, it enters the attack state, it selects a random attack for the current target and sets the active attack spec
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Attack::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    bFinished = false;
    E->GetWorldTimerManager().ClearTimer(AttackFinishHandle);
    E->ClearActiveAttackSpec();

    // ���������г���
    if (auto* Move = E->GetCharacterMovement(); Move && Move->IsFalling())
    {
        if (E->GetFSM()) E->GetFSM()->ChangeStateByName(BMEnemyStateNames::Chase);
        return;
    }

    // ����ʱֹͣ·���ƶ�����������
    E->RequestStopMovement();

    // ѡ���������
    if (!E->SelectRandomAttackForCurrentTarget(ActiveAttack))
    {
        if (E->GetFSM()) E->GetFSM()->ChangeStateByName(BMEnemyStateNames::Chase);
        return;
    }

    // д�� Combat ������
    if (UBMCombatComponent* Combat = E->GetCombat())
    {
        FBMHitBoxActivationParams Params;
        Params.AttackId = TEXT("EnemyAttack");
        Params.DamageMultiplier = 1.0f;          
        Params.bResetHitRecords = true;
        Params.DedupPolicy = EBMHitDedupPolicy::PerWindow;
        Params.MaxHitsPerTarget = 1;

        Combat->SetActiveHitBoxWindowContext(ActiveAttack.HitBoxNames, Params);
    }

    // ͬ�� EnemyBase ��ǰ��ʽ
    E->SetActiveAttackSpec(ActiveAttack);

    if (UBMCombatComponent* Combat = E->GetCombat())
    {
        Combat->SetActionLock(true);
    }

    // �� Spec ����ִ�в���
    if (ActiveAttack.bStopPathFollowingOnEnter)
    {
        E->RequestStopMovement(); // ��������
    }
    if (ActiveAttack.bFaceTargetOnEnter)
    {
        E->FaceTarget(0.f);
    }
    const float Duration = E->PlayAttackOnce(ActiveAttack);

    // �ύ����ʽ������ȴ
    if (Duration > 0.f)
    {
        if (UBMCombatComponent* Combat = E->GetCombat())
        {
            Combat->CommitCooldown(ActiveAttack.Id, ActiveAttack.Cooldown);
        }
    }

    // �ƽ���ȴ
    E->CommitAttackCooldown(ActiveAttack.Cooldown);

    if (Duration <= 0.f)
    {
        FinishAttack();
        return;
    }

    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            FinishAttack();
        });
    E->GetWorldTimerManager().SetTimer(AttackFinishHandle, D, Duration, false);
}

/*
 * @brief On exit, it exits the attack state, it clears the active attack spec and deactivates all hit boxes
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Attack::OnExit(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    if (UBMCombatComponent* Combat = E->GetCombat())
    {
        Combat->ClearActiveHitBoxWindowContext();
        Combat->SetActionLock(false);
    }
    if (UBMHitBoxComponent* HB = E->GetHitBox())
    {
        HB->DeactivateAllHitBoxes();  
    }
    E->GetWorldTimerManager().ClearTimer(AttackFinishHandle);
    E->ClearActiveAttackSpec();
    bFinished = false;
}

/*
 * @brief On update, it updates the attack state, it faces the target
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Attack::OnUpdate(float DeltaTime)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    // �����ڼ�Ҳ���Գ���ת��
    E->FaceTarget(DeltaTime);
}

/*
 * @brief Can transition to, it checks if the state can transition to the given state
 * @param StateName The name of the state to transition to
 * @return True if the state can transition to the given state, false otherwise
 */
bool UBMEnemyState_Attack::CanTransitionTo(FName StateName) const
{
    if (StateName == BMEnemyStateNames::Death)          return true;
    if (StateName == BMEnemyStateNames::Hit)            return true; 
    if (StateName == BMEnemyStateNames::PhaseChange)    return true; 
	if (StateName == BMEnemyStateNames::Dodge)          return true;
    return bFinished;
}

/*
 * @brief Finish attack, it finishes the attack, it changes the state to chase
 */
void UBMEnemyState_Attack::FinishAttack()
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    bFinished = true;

    if (E->GetFSM())
    {
        E->GetFSM()->ChangeStateByName(BMEnemyStateNames::Chase);
    }
}