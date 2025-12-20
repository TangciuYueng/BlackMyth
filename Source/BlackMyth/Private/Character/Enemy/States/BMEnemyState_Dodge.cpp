#include "Character/Enemy/States/BMEnemyState_Dodge.h"

#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMStatsComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBMEnemyState_Dodge::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    E->GetWorldTimerManager().ClearTimer(TimerHandleFinish);
    E->GetWorldTimerManager().ClearTimer(TimerHandleStep);

    bFinished = false;

    // 停止 AI 路径移动（
    E->RequestStopMovement();

    // 锁动作
    if (UBMCombatComponent* C = E->GetCombat())
    {
        C->SetActionLock(true);
    }

    // Dodge 状态：关闭所有 HurtBox（
    E->SetAllHurtBoxesEnabled(false);

    // 锁定方向：优先向后闪避
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


    // 播放闪避动画
    WindowDuration = E->PlayDodgeOnce();  
    if (WindowDuration <= 0.f)
    {
        FinishDodge();
        return;
    }

    // 设置总位移距离
    TotalDistance = FMath::Max(0.f, E->DodgeDistance);
    TraveledDistance = 0.f;

    LastStepTime = E->GetWorld()->GetTimeSeconds();

    // 开启 Step 推进
    E->GetWorldTimerManager().SetTimer(
        TimerHandleStep,
        this,
        &UBMEnemyState_Dodge::StepDodge,
        1.0f / 60.0f,
        true
    );

    // 动画结束
    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]() { FinishDodge(); });
    E->GetWorldTimerManager().SetTimer(TimerHandleFinish, D, WindowDuration, false);
}

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

    // 平台边缘保护：下一步脚下没地就停止位移
    if (!HasWalkableFloorAt(Next, E))
    {
        E->GetWorldTimerManager().ClearTimer(TimerHandleStep);
        return;
    }

    // Sweep 推进
    FHitResult Hit;
    E->SetActorLocation(Next, true, &Hit, ETeleportType::None);

    const float Moved = FVector::Dist2D(Cur, E->GetActorLocation());
    TraveledDistance += Moved;

    if (TraveledDistance >= TotalDistance - 1.0f)
    {
        E->GetWorldTimerManager().ClearTimer(TimerHandleStep);
    }
}

void UBMEnemyState_Dodge::OnExit(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    E->GetWorldTimerManager().ClearTimer(TimerHandleFinish);
    E->GetWorldTimerManager().ClearTimer(TimerHandleStep);

    // 恢复 HurtBox
    E->SetAllHurtBoxesEnabled(true);

    // 解锁
    if (UBMCombatComponent* C = E->GetCombat())
    {
        C->SetActionLock(false);
    }

    bFinished = false;
}

bool UBMEnemyState_Dodge::CanTransitionTo(FName StateName) const
{
    // Dodge 不可被打断
    if (StateName == BMEnemyStateNames::Death) return true;
    return bFinished;
}

void UBMEnemyState_Dodge::FinishDodge()
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    bFinished = true;

    // 退出 Dodge 后回到 Idle 或 Chase（
    if (UBMStateMachineComponent* FSM = E->GetFSM())
    {
        FSM->ChangeStateByName(E->IsAlerted() ? BMEnemyStateNames::Chase : BMEnemyStateNames::Idle);
    }
}
