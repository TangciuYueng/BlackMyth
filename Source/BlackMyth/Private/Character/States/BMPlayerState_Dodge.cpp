#include "Character/States/BMPlayerState_Dodge.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBMPlayerState_Dodge::OnEnter(float)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->GetWorldTimerManager().ClearTimer(TimerHandleFinish);
    PC->GetWorldTimerManager().ClearTimer(TimerHandleStep);

    // 冷却
    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        if (!Combat->TryCommitCooldown(PC->DodgeCooldownKey, PC->DodgeCooldown))
        {
            // 冷却未好，直接回退
            if (UBMStateMachineComponent* M = PC->GetFSM())
                M->ChangeStateByName(PC->HasMoveIntent() ? BMStateNames::Move : BMStateNames::Idle);
            return;
        }

        // 闪避期间锁动作
        Combat->SetActionLock(true);
    }

    bFinished = false;

    // 关闭所有 HurtBox
    PC->SetAllHurtBoxesEnabled(false);

    // 锁定方向
    LockedDir = PC->ComputeDodgeDirectionLocked();
    LockedDir.Z = 0.f;
    LockedDir = LockedDir.IsNearlyZero() ? PC->GetActorForwardVector() : LockedDir.GetSafeNormal();

    // 禁用朝移动方向转向
    if (UCharacterMovementComponent* Move = PC->GetCharacterMovement())
    {
        bSavedOrientToMovement = Move->bOrientRotationToMovement;
        SavedMaxWalkSpeed = Move->MaxWalkSpeed;

        Move->bOrientRotationToMovement = false;
        Move->MaxWalkSpeed = FMath::Max(SavedMaxWalkSpeed, PC->DodgeSpeed);

        Move->StopMovementImmediately();
    }


    // 播放动画
    WindowDuration = PC->PlayDodgeOnce(); 
    if (WindowDuration <= 0.f)
    {
        FinishDodge();
        return;
    }
    // 设置总位移
    TotalDistance = FMath::Max(0.f, PC->DodgeDistance);
    TraveledDistance = 0.f;

    // 初始化步进时间
    LastStepTime = PC->GetWorld()->GetTimeSeconds();

    // 开启位移步进
    PC->GetWorldTimerManager().SetTimer(
        TimerHandleStep,
        this,
        &UBMPlayerState_Dodge::StepDodge,
        1.0f / 60.0f,
        true
    );

    // 动画结束 -> 结束状态
    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]() { FinishDodge(); });
    PC->GetWorldTimerManager().SetTimer(TimerHandleFinish, D, WindowDuration, false);
}

bool UBMPlayerState_Dodge::HasWalkableFloorAt(const FVector& WorldPos, const ACharacter* Char) const
{
    if (!Char) return false;
    UWorld* W = Char->GetWorld();
    if (!W) return false;

    const UCharacterMovementComponent* Move = Char->GetCharacterMovement();
    const float WalkableZ = Move ? Move->GetWalkableFloorZ() : 0.7f; // 默认斜率阈值兜底

    FCollisionQueryParams Q;
    Q.bTraceComplex = false;
    Q.AddIgnoredActor(Char);

    const FVector Start = WorldPos + FVector(0, 0, LedgeProbeUp);
    const FVector End = WorldPos - FVector(0, 0, LedgeProbeDown);

    FHitResult Hit;
    const bool bHit = W->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Q);
    if (!bHit) return false;

    // 法线 Z 太小说明太陡/不可走
    return Hit.ImpactNormal.Z >= WalkableZ;
}

void UBMPlayerState_Dodge::StepDodge()
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC || bFinished) return;

    UWorld* W = PC->GetWorld();
    if (!W) return;

    const float Now = W->GetTimeSeconds();
    float Dt = Now - LastStepTime;
    LastStepTime = Now;

    Dt = FMath::Clamp(Dt, 0.f, 0.05f); // 防止卡顿时一步走太远

    if (TotalDistance <= 0.f || WindowDuration <= 0.f)
    {
        // 不需要位移
        PC->GetWorldTimerManager().ClearTimer(TimerHandleStep);
        return;
    }

    // 速度由总位移/总时间推导
    const float Speed = TotalDistance / WindowDuration;
    float Step = Speed * Dt;

    const float Remaining = TotalDistance - TraveledDistance;
    Step = FMath::Min(Step, Remaining);
    if (Step <= KINDA_SMALL_NUMBER)
    {
        PC->GetWorldTimerManager().ClearTimer(TimerHandleStep);
        return;
    }

    const FVector Cur = PC->GetActorLocation();
    const FVector Delta = LockedDir * Step;
    const FVector Next = Cur + Delta;

    // 平台边缘保护 —— 下一步脚下没有可走地面就停止位移
    if (!HasWalkableFloorAt(Next, PC))
    {
        PC->GetWorldTimerManager().ClearTimer(TimerHandleStep);
        return;
    }

    // 遇到墙/障碍会阻挡
    FHitResult Hit;
    PC->SetActorLocation(Next, true, &Hit, ETeleportType::None);

    // 统计实际位移
    const float Moved = FVector::Dist2D(Cur, PC->GetActorLocation());
    TraveledDistance += Moved;

    // 达到总位移则停止位移推进
    if (TraveledDistance >= TotalDistance - 1.0f)
    {
        PC->GetWorldTimerManager().ClearTimer(TimerHandleStep);
    }
}

void UBMPlayerState_Dodge::OnExit(float)
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    PC->GetWorldTimerManager().ClearTimer(TimerHandleFinish);
    PC->GetWorldTimerManager().ClearTimer(TimerHandleStep);

    // 恢复 HurtBox
    PC->SetAllHurtBoxesEnabled(true);

    // 解锁动作
    if (UBMCombatComponent* Combat = PC->GetCombat())
    {
        Combat->SetActionLock(false);
    }

    // 恢复移动参数
    if (UCharacterMovementComponent* Move = PC->GetCharacterMovement())
    {
        Move->bOrientRotationToMovement = bSavedOrientToMovement;
        Move->MaxWalkSpeed = SavedMaxWalkSpeed;
    }

    bFinished = false;
}

bool UBMPlayerState_Dodge::CanTransitionTo(FName StateName) const
{
    // 不可被打断
    if (!bFinished)
    {
        return (StateName == BMStateNames::Death);
    }
    return true;
}

void UBMPlayerState_Dodge::FinishDodge()
{
    ABMPlayerCharacter* PC = Cast<ABMPlayerCharacter>(GetContext());
    if (!PC) return;

    bFinished = true;

    if (UBMStateMachineComponent* M = PC->GetFSM())
    {
        M->ChangeStateByName(PC->HasMoveIntent() ? BMStateNames::Move : BMStateNames::Idle);
    }
}
