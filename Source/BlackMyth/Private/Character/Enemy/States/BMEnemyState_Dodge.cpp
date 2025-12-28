#include "Character/Enemy/States/BMEnemyState_Dodge.h"

#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMStatsComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

/*
 * @brief On enter, it enters the dodge state, it stops the enemy movement, deactivates all hurt boxes and sets the action lock to true
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Dodge::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    E->GetWorldTimerManager().ClearTimer(TimerHandleFinish);
    E->GetWorldTimerManager().ClearTimer(TimerHandleStep);

    bFinished = false;

    // ֹͣ AI ·���ƶ�
    E->RequestStopMovement();

    // ������
    if (UBMCombatComponent* C = E->GetCombat())
    {
        C->SetActionLock(true);
    }

    // Dodge ״̬�ر����� HurtBox
    E->SetAllHurtBoxesEnabled(false);

    // �������������������
    FVector Dir = -E->GetActorForwardVector();
    if (const AActor* Inst = E->GetLastDamageInfo().InstigatorActor.Get())
    {
        const FVector ToInst = (Inst->GetActorLocation() - E->GetActorLocation());
        Dir = (-ToInst);
    }
    else if (APawn* T = E->GetCurrentTarget())
    {
        const FVector ToTarget = (T->GetActorLocation() - E->GetActorLocation());
        Dir = (-ToTarget);
    }
    Dir.Z = 0.f;
    LockedDir = Dir.IsNearlyZero() ? (-E->GetActorForwardVector()) : Dir.GetSafeNormal();


    // �������ܶ���
    WindowDuration = E->PlayDodgeOnce();  
    if (WindowDuration <= 0.f)
    {
        FinishDodge();
        return;
    }

    // ������λ�ƾ���
    TotalDistance = FMath::Max(0.f, E->DodgeDistance);
    TraveledDistance = 0.f;

    LastStepTime = E->GetWorld()->GetTimeSeconds();

    // ���� Step �ƽ�
    E->GetWorldTimerManager().SetTimer(
        TimerHandleStep,
        this,
        &UBMEnemyState_Dodge::StepDodge,
        1.0f / 60.0f,
        true
    );

    // ��������
    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]() { FinishDodge(); });
    E->GetWorldTimerManager().SetTimer(TimerHandleFinish, D, WindowDuration, false);
}

/*
 * @brief Has walkable floor at, it checks if the character has a walkable floor at the given world position
 * @param WorldPos The world position
 * @param Char The character
 * @return True if the character has a walkable floor at the given world position, false otherwise
 */
bool UBMEnemyState_Dodge::HasWalkableFloorAt(const FVector& WorldPos, const ACharacter* Char) const
{
    if (!Char) return false;
    UWorld* W = Char->GetWorld();
    if (!W) return false;

    const UCharacterMovementComponent* Move = Char->GetCharacterMovement();
    const float WalkableZ = Move ? Move->GetWalkableFloorZ() : 0.7f;

    FCollisionQueryParams Q;
    Q.bTraceComplex = false;
    Q.AddIgnoredActor(Char);

    const FVector Start = WorldPos + FVector(0, 0, LedgeProbeUp);
    const FVector End = WorldPos - FVector(0, 0, LedgeProbeDown);

    FHitResult Hit;
    const bool bHit = W->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Q);
    if (!bHit) return false;

    return Hit.ImpactNormal.Z >= WalkableZ;
}

/*
 * @brief Step dodge, it steps the dodge
 */
void UBMEnemyState_Dodge::StepDodge()
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E || bFinished) return;

    UWorld* W = E->GetWorld();
    if (!W) return;

    const float Now = W->GetTimeSeconds();
    float Dt = Now - LastStepTime;
    LastStepTime = Now;

    Dt = FMath::Clamp(Dt, 0.f, 0.05f);

    if (TotalDistance <= 0.f || WindowDuration <= 0.f)
    {
        E->GetWorldTimerManager().ClearTimer(TimerHandleStep);
        return;
    }

    const float Speed = TotalDistance / WindowDuration;
    float Step = Speed * Dt;

    const float Remaining = TotalDistance - TraveledDistance;
    Step = FMath::Min(Step, Remaining);
    if (Step <= KINDA_SMALL_NUMBER)
    {
        E->GetWorldTimerManager().ClearTimer(TimerHandleStep);
        return;
    }

    const FVector Cur = E->GetActorLocation();
    const FVector Delta = LockedDir * Step;
    const FVector Next = Cur + Delta;

    // ƽ̨��Ե����
    if (!HasWalkableFloorAt(Next, E))
    {
        E->GetWorldTimerManager().ClearTimer(TimerHandleStep);
        return;
    }

    // Sweep �ƽ�
    FHitResult Hit;
    E->SetActorLocation(Next, true, &Hit, ETeleportType::None);

    const float Moved = FVector::Dist2D(Cur, E->GetActorLocation());
    TraveledDistance += Moved;

    if (TraveledDistance >= TotalDistance - 1.0f)
    {
        E->GetWorldTimerManager().ClearTimer(TimerHandleStep);
    }
}

/*
 * @brief On exit, it exits the dodge state, it clears the timers, enables all hurt boxes and sets the action lock to false
 * @param DeltaTime The delta time
 */
void UBMEnemyState_Dodge::OnExit(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    E->GetWorldTimerManager().ClearTimer(TimerHandleFinish);
    E->GetWorldTimerManager().ClearTimer(TimerHandleStep);

    // �ָ� HurtBox
    E->SetAllHurtBoxesEnabled(true);

    // ����
    if (UBMCombatComponent* C = E->GetCombat())
    {
        C->SetActionLock(false);
    }

    bFinished = false;
}

/*
 * @brief Can transition to, it checks if the state can transition to the given state
 * @param StateName The name of the state to transition to
 * @return True if the state can transition to the given state, false otherwise
 */
bool UBMEnemyState_Dodge::CanTransitionTo(FName StateName) const
{
    // Dodge ���ɱ����
    if (StateName == BMEnemyStateNames::Death) return true;
    return bFinished;
}

/*
 * @brief Finish dodge, it finishes the dodge, it changes the state to chase or idle
 */
void UBMEnemyState_Dodge::FinishDodge()
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    bFinished = true;

    // �˳� Dodge ��ص� Idle �� Chase
    if (UBMStateMachineComponent* FSM = E->GetFSM())
    {
        FSM->ChangeStateByName(E->IsAlerted() ? BMEnemyStateNames::Chase : BMEnemyStateNames::Idle);
    }
}
