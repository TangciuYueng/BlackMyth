#include "Character/Enemy/States/BMEnemyState_Attack.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMCombatComponent.h"
#include "Core/BMTypes.h"
#include "GameFramework/CharacterMovementComponent.h"


void UBMEnemyState_Attack::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    bFinished = false;
    E->GetWorldTimerManager().ClearTimer(AttackFinishHandle);
    E->ClearActiveAttackSpec();

    // 不允许空中出手：直接回追击
    if (auto* Move = E->GetCharacterMovement(); Move && Move->IsFalling())
    {
        if (E->GetFSM()) E->GetFSM()->ChangeStateByName(BMEnemyStateNames::Chase);
        return;
    }

    // 出手时停止路径移动（保留惯性，不用 StopMovementImmediately）
    E->RequestStopMovement();

    // 选择随机攻击
    if (!E->SelectRandomAttackForCurrentTarget(ActiveAttack))
    {
        if (E->GetFSM()) E->GetFSM()->ChangeStateByName(BMEnemyStateNames::Chase);
        return;
    }

    // 1) 写入 Combat 上下文：给 NotifyState 使用（开启哪些 HitBox）
    if (UBMCombatComponent* Combat = E->FindComponentByClass<UBMCombatComponent>())
    {
        FBMHitBoxActivationParams Params;
        Params.AttackId = TEXT("EnemyAttack");
        Params.DamageMultiplier = 1.0f;          // 以后你可从 Spec 扩展 DamageMultiplier
        Params.bResetHitRecords = true;
        Params.DedupPolicy = EBMHitDedupPolicy::PerWindow;
        Params.MaxHitsPerTarget = 1;

        Combat->SetActiveHitBoxWindowContext(ActiveAttack.HitBoxNames, Params);
    }

    // 2) 同步 EnemyBase 当前招式
    E->SetActiveAttackSpec(ActiveAttack);

    // 按 Spec 控制执行策略（工程性：以后扩展技能/霸体无需改 State）
    if (ActiveAttack.bStopPathFollowingOnEnter)
    {
        E->RequestStopMovement(); // 保留惯性（不是 StopMovementImmediately）
    }
    if (ActiveAttack.bFaceTargetOnEnter)
    {
        E->FaceTarget(0.f);
    }
    const float Duration = E->PlayAttackOnce(ActiveAttack);

    // 推进冷却（由 Spec 决定）
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

void UBMEnemyState_Attack::OnExit(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    if (UBMCombatComponent* Combat = E->FindComponentByClass<UBMCombatComponent>())
    {
        Combat->ClearActiveHitBoxWindowContext();
    }
    E->GetWorldTimerManager().ClearTimer(AttackFinishHandle);
    E->ClearActiveAttackSpec();
    bFinished = false;
}

void UBMEnemyState_Attack::OnUpdate(float DeltaTime)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    // 攻击期间也可以持续转向（可选）
    E->FaceTarget(DeltaTime);
}

bool UBMEnemyState_Attack::CanTransitionTo(FName StateName) const
{
    if (StateName == BMEnemyStateNames::Death) return true;
    if (StateName == BMEnemyStateNames::Hit)   return true; // 是否真的进 Hit，由 EnemyBase 决策
    return bFinished;
}

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