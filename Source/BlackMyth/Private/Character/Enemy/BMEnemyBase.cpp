#include "Character/Enemy/BMEnemyBase.h"

#include "Character/Enemy/BMEnemyAIController.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStatsComponent.h"

#include "Character/Enemy/States/BMEnemyState_Idle.h"
#include "Character/Enemy/States/BMEnemyState_Patrol.h"
#include "Character/Enemy/States/BMEnemyState_Chase.h"
#include "Character/Enemy/States/BMEnemyState_Attack.h"
#include "Character/Enemy/States/BMEnemyState_Hit.h"
#include "Character/Enemy/States/BMEnemyState_Death.h"

#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ABMEnemyBase::ABMEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;
    CharacterType = EBMCharacterType::Enemy;
    Team = EBMTeam::Enemy;

    AIControllerClass = ABMEnemyAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // 与 Player 一致：先用 SingleNode（你后续换 AnimInstance 也不影响 FSM 结构）
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bOrientRotationToMovement = true;
        Move->RotationRate = FRotator(0.f, 720.f, 0.f);
        Move->MaxWalkSpeed = PatrolSpeed;
    }
}

void ABMEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    HomeLocation = GetActorLocation();

    CachePlayerPawn();
    StartPerceptionTimer();

    InitEnemyStates();

}

void ABMEnemyBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 防止“卡住/没路径/AcceptanceRadius过大”等导致原地播Walk/Run
    // UpdateLocomotionLoop();
}

void ABMEnemyBase::InitEnemyStates()
{
    UBMStateMachineComponent* Machine = GetFSM();
    if (!Machine) return;

    auto* SIdle = NewObject<UBMEnemyState_Idle>(Machine);
    auto* SPatrol = NewObject<UBMEnemyState_Patrol>(Machine);
    auto* SChase = NewObject<UBMEnemyState_Chase>(Machine);
    auto* SAtk = NewObject<UBMEnemyState_Attack>(Machine);
    auto* SHit = NewObject<UBMEnemyState_Hit>(Machine);
    auto* SDeath = NewObject<UBMEnemyState_Death>(Machine);

    SIdle->Init(this);
    SPatrol->Init(this);
    SChase->Init(this);
    SAtk->Init(this);
    SHit->Init(this);
    SDeath->Init(this);

    Machine->RegisterState(BMEnemyStateNames::Idle, SIdle);
    Machine->RegisterState(BMEnemyStateNames::Patrol, SPatrol);
    Machine->RegisterState(BMEnemyStateNames::Chase, SChase);
    Machine->RegisterState(BMEnemyStateNames::Attack, SAtk);
    Machine->RegisterState(BMEnemyStateNames::Hit, SHit);
    Machine->RegisterState(BMEnemyStateNames::Death, SDeath);

    Machine->ChangeStateByName(BMEnemyStateNames::Idle);
}

void ABMEnemyBase::CachePlayerPawn()
{
    if (UWorld* W = GetWorld())
    {
        CachedPlayer = UGameplayStatics::GetPlayerPawn(W, 0);
    }
}

void ABMEnemyBase::StartPerceptionTimer()
{
    if (!GetWorld() || PerceptionInterval <= 0.f) return;

    GetWorldTimerManager().SetTimer(
        PerceptionTimerHandle,
        this,
        &ABMEnemyBase::UpdatePerception,
        PerceptionInterval,
        true
    );
}

void ABMEnemyBase::UpdatePerception()
{
    const bool bDetected = DetectPlayer();
    SetAlertState(bDetected);

    if (bDetected)
    {
        CurrentTarget = CachedPlayer.Get();
    }
    else
    {
        CurrentTarget = nullptr;
    }
}

bool ABMEnemyBase::DetectPlayer() const
{
    APawn* PlayerPawn = CachedPlayer.Get();
    if (!PlayerPawn)
    {
        if (UWorld* W = GetWorld())
        {
            PlayerPawn = UGameplayStatics::GetPlayerPawn(W, 0);
            const_cast<ABMEnemyBase*>(this)->CachedPlayer = PlayerPawn;
        }
    }
    if (!PlayerPawn || AggroRange <= 0.f) return false;

    const float DistSq = FVector::DistSquared(PlayerPawn->GetActorLocation(), GetActorLocation());
    return DistSq <= FMath::Square(AggroRange);
}

void ABMEnemyBase::SetAlertState(bool bAlert)
{
    if (bIsAlert == bAlert) return;
    bIsAlert = bAlert;
}

void ABMEnemyBase::DropLoot()
{
    for (const FBMLootItem& Item : LootTable)
    {
        if (Item.ItemID.IsNone()) continue;
        const float P = FMath::Clamp(Item.Probability, 0.f, 1.f);
        if (FMath::FRand() > P) continue;

        const int32 MinQ = FMath::Max(1, Item.MinQuantity);
        const int32 MaxQ = FMath::Max(MinQ, Item.MaxQuantity);
        const int32 Qty = FMath::RandRange(MinQ, MaxQ);

        // 基类只负责“决定掉不掉、掉多少”，具体 Spawn 交给掉落系统/派生类
        UE_LOG(LogTemp, Log, TEXT("[%s] DropLoot: %s x%d"), *GetName(), *Item.ItemID.ToString(), Qty);
    }
}

bool ABMEnemyBase::IsInAttackRange() const
{
    APawn* T = CurrentTarget.Get();
    if (!T) return false;

    const float Dist2D = FVector::Dist2D(T->GetActorLocation(), GetActorLocation());

    if (AttackRangeOverride >= 0.f)
    {
        return Dist2D <= AttackRangeOverride;
    }

    // 无 override：只要存在一个 attack spec 能覆盖当前距离即可
    for (const FBMEnemyAttackSpec& S : AttackSpecs)
    {
        if (!S.Anim) continue;
        if (Dist2D >= S.MinRange && Dist2D <= S.MaxRange)
        {
            return true;
        }
    }
    return false;
}

bool ABMEnemyBase::CanStartAttack() const
{
    if (!HasValidTarget()) return false;

    // 不允许空中出手（即便敌人被击退到空中）
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        if (Move->IsFalling()) return false;
    }

    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    return Now >= NextAttackAllowedTime && IsInAttackRange();
}

void ABMEnemyBase::CommitAttackCooldown(float CooldownSeconds)
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    NextAttackAllowedTime = Now + FMath::Max(0.f, CooldownSeconds);
}

bool ABMEnemyBase::SelectRandomAttackForCurrentTarget(FBMEnemyAttackSpec& OutSpec) const
{
    APawn* T = CurrentTarget.Get();
    if (!T) return false;

    const float Dist2D = FVector::Dist2D(T->GetActorLocation(), GetActorLocation());

    // 过滤可用攻击
    TArray<const FBMEnemyAttackSpec*> Candidates;
    float TotalW = 0.f;

    for (const FBMEnemyAttackSpec& S : AttackSpecs)
    {
        if (!S.Anim) continue;
        if (Dist2D < S.MinRange || Dist2D > S.MaxRange) continue;

        const float W = FMath::Max(0.01f, S.Weight);
        Candidates.Add(&S);
        TotalW += W;
    }

    if (Candidates.Num() == 0) return false;

    float R = FMath::FRandRange(0.f, TotalW);
    for (const FBMEnemyAttackSpec* S : Candidates)
    {
        R -= FMath::Max(0.01f, S->Weight);
        if (R <= 0.f)
        {
            OutSpec = *S;
            return true;
        }
    }

    OutSpec = *Candidates.Last();
    return true;
}

void ABMEnemyBase::SetSingleNodePlayRate(float Rate)
{
    if (!GetMesh()) return;

    Rate = FMath::Max(0.01f, Rate);

    if (UAnimSingleNodeInstance* Inst = GetMesh()->GetSingleNodeInstance())
    {
        Inst->SetPlayRate(Rate);
    }
}

void ABMEnemyBase::PlayLoop(UAnimSequence* Seq, float PlayRate)
{
    if (!Seq || !GetMesh()) return;

    PlayRate = FMath::Max(0.01f, PlayRate);

    const bool bSameAnim = (CurrentLoopAnim == Seq);
    const bool bSameRate = FMath::IsNearlyEqual(CurrentLoopRate, PlayRate, 0.001f);

    // 同一动画 + 同一速率：不重复下发
    if (bSameAnim && bSameRate)
    {
        return;
    }

    CurrentLoopAnim = Seq;
    CurrentLoopRate = PlayRate;

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    GetMesh()->PlayAnimation(Seq, true);
    SetSingleNodePlayRate(PlayRate);
}

float ABMEnemyBase::PlayOnce(UAnimSequence* Seq, float PlayRate)
{
    if (!Seq || !GetMesh()) return 0.f;

    PlayRate = FMath::Max(0.01f, PlayRate);

    // OneShot 会打断 loop：必须清空 loop 缓存，避免后续 PlayIdleLoop 被错误短路
    CurrentLoopAnim = nullptr;
    CurrentLoopRate = 1.0f;

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    GetMesh()->PlayAnimation(Seq, false);
    SetSingleNodePlayRate(PlayRate);

    return Seq->GetPlayLength() / PlayRate;
}

void ABMEnemyBase::PlayIdleLoop()
{
    PlayLoop(AnimIdle, 1.0f);
}

void ABMEnemyBase::PlayWalkLoop()
{
    PlayLoop(AnimWalk, 1.0f);
}

void ABMEnemyBase::PlayRunLoop()
{
    PlayLoop(AnimRun, 1.0f);
}

float ABMEnemyBase::PlayAttackOnce(const FBMEnemyAttackSpec& Spec)
{
    return PlayOnce(Spec.Anim, Spec.PlayRate);
}

float ABMEnemyBase::PlayHitOnce(const FBMDamageInfo& Info)
{
    UAnimSequence* Seq = nullptr;

    if (BMCombatUtils::IsHeavyIncoming(Info))
        Seq = AnimHitHeavy ? AnimHitHeavy : AnimHitLight;
    else
        Seq = AnimHitLight ? AnimHitLight : AnimHitHeavy;

    return PlayOnce(Seq, 1.0f);
}

float ABMEnemyBase::PlayDeathOnce()
{
    return PlayOnce(AnimDeath, 1.0f);
}

bool ABMEnemyBase::RequestMoveToTarget(float AcceptanceRadius)
{
    ABMEnemyAIController* C = Cast<ABMEnemyAIController>(GetController());
    if (!C || !HasValidTarget()) return false;
    return C->RequestMoveToActor(CurrentTarget.Get(), AcceptanceRadius);
}

bool ABMEnemyBase::RequestMoveToLocation(const FVector& Location, float AcceptanceRadius)
{
    ABMEnemyAIController* C = Cast<ABMEnemyAIController>(GetController());
    if (!C) return false;
    return C->RequestMoveToLocation(Location, AcceptanceRadius);
}

void ABMEnemyBase::RequestStopMovement()
{
    if (ABMEnemyAIController* C = Cast<ABMEnemyAIController>(GetController()))
    {
        C->RequestStopMovement();
    }
}

void ABMEnemyBase::FaceTarget(float DeltaSeconds, float TurnSpeedDeg)
{
    APawn* T = CurrentTarget.Get();
    if (!T) return;

    const FVector To = (T->GetActorLocation() - GetActorLocation());
    const FRotator Want(0.f, To.Rotation().Yaw, 0.f);

    const float Alpha = FMath::Clamp(TurnSpeedDeg * DeltaSeconds / 360.f, 0.f, 1.f);
    SetActorRotation(FMath::Lerp(GetActorRotation(), Want, Alpha));
}

bool ABMEnemyBase::CanBeDamagedBy(const FBMDamageInfo& Info) const
{
    if (!Super::CanBeDamagedBy(Info)) return false;

    if (const ABMCharacterBase* Inst = Cast<ABMCharacterBase>(Info.InstigatorActor.Get()))
    {
        if (Inst->Team == Team) return false;
    }
    return true;
}

void ABMEnemyBase::SetActiveAttackSpec(const FBMEnemyAttackSpec& Spec)
{
    ActiveAttackSpec = Spec;
    bHasActiveAttackSpec = true;
}

void ABMEnemyBase::ClearActiveAttackSpec()
{
    bHasActiveAttackSpec = false;
}

bool ABMEnemyBase::ShouldInterruptCurrentAttack(const FBMDamageInfo& Incoming) const
{
    if (!bHasActiveAttackSpec)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] InterruptCheck: No ActiveAttackSpec while attacking. Default=Interruptible."),
            *GetName());
        return true; // 没有招式信息：默认可打断
    }

    const FBMEnemyAttackSpec& Spec = ActiveAttackSpec;
    if (Spec.bUninterruptible)
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] InterruptCheck: Spec is Uninterruptible. Attack will NOT be interrupted."),
            *GetName());
        return false; // 霸体：不可打断
    }

    const float P = BMCombatUtils::IsHeavyIncoming(Incoming) ? Spec.InterruptChanceOnHeavyHit : Spec.InterruptChance;
    const float ClampedP = FMath::Clamp(P, 0.f, 1.f);
    const float R = FMath::FRand();
    const bool bWillInterrupt = (R < ClampedP);

    // 说明当前是什么招式、打断概率多少，这次是否打断成功
    UE_LOG(
        LogTemp,
        Log,
        TEXT("[%s] InterruptCheck: Attack=%s, Incoming=%s, Chance=%.2f, Rand=%.2f, Result=%s"),
        *GetName(),
        Spec.Anim ? *Spec.Anim->GetName() : TEXT("None"),
        BMCombatUtils::IsHeavyIncoming(Incoming) ? TEXT("Heavy") : TEXT("Normal"),
        ClampedP,
        R,
        bWillInterrupt ? TEXT("INTERRUPTED") : TEXT("NOT_INTERRUPTED")
    );

    return bWillInterrupt;
}

void ABMEnemyBase::RequestHitState(const FBMDamageInfo& FinalInfo)
{
    LastDamageInfo = FinalInfo;

    UBMStateMachineComponent* Machine = GetFSM();
    if (!Machine) return;

    // 死亡状态优先
    if (Machine->GetCurrentStateName() == BMEnemyStateNames::Death)
    {
        return;
    }

    const FName Cur = Machine->GetCurrentStateName();

    // 非攻击：必进受击
    if (Cur != BMEnemyStateNames::Attack)
    {
        Machine->ChangeStateByName(BMEnemyStateNames::Hit);
        return;
    }

    // 攻击中：看当前招式是否可打断
    if (ShouldInterruptCurrentAttack(FinalInfo))
    {
        Machine->ChangeStateByName(BMEnemyStateNames::Hit);
    }
    // 否则保持攻击（只扣血不播受击）
}

void ABMEnemyBase::RequestDeathState(const FBMDamageInfo& LastHitInfo)
{
    LastDamageInfo = LastHitInfo;

    UBMStateMachineComponent* Machine = GetFSM();
    if (!Machine) return;

    Machine->ChangeStateByName(BMEnemyStateNames::Death);
}

// ===== 动画播放（SingleNode）=====

static void ApplySingleNodePlayRate(USkeletalMeshComponent* Mesh, float Rate)
{
    if (!Mesh) return;
    if (UAnimSingleNodeInstance* Inst = Mesh->GetSingleNodeInstance())
    {
        Inst->SetPlayRate(FMath::Max(0.01f, Rate));
    }
}

// ===== 在伤害链路里触发 Hit/Death 状态 =====
void ABMEnemyBase::HandleDamageTaken(const FBMDamageInfo& FinalInfo)
{
    Super::HandleDamageTaken(FinalInfo);

    // 若已经死了，HandleDeath 会接管
    if (UBMStatsComponent* S = GetStats())
    {
        if (S->IsDead()) return;
    }

    RequestHitState(FinalInfo);
}

void ABMEnemyBase::HandleDeath(const FBMDamageInfo& LastHitInfo)
{
    // 先切 Death 状态，再做通用逻辑（避免 Super 停止动画/销毁导致播不出来）
    RequestDeathState(LastHitInfo);

    Super::HandleDeath(LastHitInfo);
    DropLoot();
}

bool ABMEnemyBase::ResolveHitBoxWindow(
    FName WindowId,
    TArray<FName>& OutHitBoxNames,
    FBMHitBoxActivationParams& OutParams
) const
{
    OutHitBoxNames.Reset();
    OutParams = FBMHitBoxActivationParams();

    if (!bHasActiveAttackSpec)
    {
        return false;
    }

    static const FName DefaultWindowId(TEXT("HitWindow"));

    // 目前你 Enemy Spec 也建议用“单窗口”字段（HitBoxNames/HitBoxParams）
    if (WindowId.IsNone() || WindowId == DefaultWindowId)
    {
        OutHitBoxNames = ActiveAttackSpec.HitBoxNames;
        OutParams = ActiveAttackSpec.HitBoxParams;
        return OutHitBoxNames.Num() > 0;
    }

    // 后续如果做多窗口（ActiveAttackSpec.Windows），在这里补 WindowId 查找即可
    return false;
}