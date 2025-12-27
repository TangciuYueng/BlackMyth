#include "Character/Enemy/BMEnemyBase.h"

#include "Character/Enemy/BMEnemyAIController.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMInventoryComponent.h"
#include "Character/Components/BMExperienceComponent.h"
#include "Character/Components/BMHealthBarComponent.h"
#include "Components/CapsuleComponent.h"

#include "Character/Enemy/States/BMEnemyState_Idle.h"
#include "Character/Enemy/States/BMEnemyState_Patrol.h"
#include "Character/Enemy/States/BMEnemyState_Chase.h"
#include "Character/Enemy/States/BMEnemyState_Attack.h"
#include "Character/Enemy/States/BMEnemyState_Hit.h"
#include "Character/Enemy/States/BMEnemyState_Death.h"
#include "Character/Enemy/States/BMEnemyState_Dodge.h"

#include "System/BMEnemyManagerSubsystem.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "Core/BMDataSubsystem.h"

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
    InitFloatingHealthBar();

    // 注册到敌人管理子系统
    if (UWorld* World = GetWorld())
    {
        if (UBMEnemyManagerSubsystem* EnemyManager = World->GetSubsystem<UBMEnemyManagerSubsystem>())
        {
            EnemyManager->RegisterEnemy(this);
        }
    }
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
    auto* SDodge = NewObject<UBMEnemyState_Dodge>(Machine);
    
    SIdle->Init(this);
    SPatrol->Init(this);
    SChase->Init(this);
    SAtk->Init(this);
    SHit->Init(this);
    SDeath->Init(this);
    SDodge->Init(this);

    Machine->RegisterState(BMEnemyStateNames::Idle, SIdle);
    Machine->RegisterState(BMEnemyStateNames::Patrol, SPatrol);
    Machine->RegisterState(BMEnemyStateNames::Chase, SChase);
    Machine->RegisterState(BMEnemyStateNames::Attack, SAtk);
    Machine->RegisterState(BMEnemyStateNames::Hit, SHit);
    Machine->RegisterState(BMEnemyStateNames::Death, SDeath);
    Machine->RegisterState(BMEnemyStateNames::Dodge, SDodge);

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

    // 玩家死亡
    if (const ABMCharacterBase* PlayerChar = Cast<ABMCharacterBase>(PlayerPawn))
    {
        if (PlayerChar->GetStats()->IsDead()) return false;   
    }

    const float DistSq = FVector::DistSquared(PlayerPawn->GetActorLocation(), GetActorLocation());
    return DistSq <= FMath::Square(AggroRange);
}

void ABMEnemyBase::SetAlertState(bool bAlert)
{
    if (bIsAlert == bAlert) return;
    bIsAlert = bAlert;

    if (FloatingHealthBar)
    {
        FloatingHealthBar->SetVisibility(bAlert, true);
    }
}

void ABMEnemyBase::DropLoot()
{
    // 获取玩家
    APawn* PlayerPawn = CachedPlayer.Get();
    if (!PlayerPawn)
    {
        if (UWorld* W = GetWorld())
        {
            PlayerPawn = UGameplayStatics::GetPlayerPawn(W, 0);
        }
    }

    if (!PlayerPawn)
    {
        return;
    }

    // 获取玩家的 Inventory 和 Experience 组件
    UBMInventoryComponent* PlayerInventory = PlayerPawn->FindComponentByClass<UBMInventoryComponent>();
    UBMExperienceComponent* PlayerExperience = PlayerPawn->FindComponentByClass<UBMExperienceComponent>();

    int32 TotalCurrency = 0;
    float TotalExp = 0.0f;
    TArray<FString> DroppedItems;

    // 掉落金币
    if (PlayerInventory && CurrencyDropMax > 0)
    {
        const int32 Currency = FMath::RandRange(FMath::Max(0, CurrencyDropMin), FMath::Max(0, CurrencyDropMax));
        if (Currency > 0)
        {
            if (PlayerInventory->AddCurrency(Currency))
            {
                TotalCurrency = Currency;
            }
        }
    }

    // 掉落经验
    if (PlayerExperience && ExpDropMax > 0.0f)
    {
        const float Exp = FMath::FRandRange(FMath::Max(0.0f, ExpDropMin), FMath::Max(0.0f, ExpDropMax));
        if (Exp > 0.0f)
        {
            PlayerExperience->AddXP(Exp);
            TotalExp = Exp;
        }
    }

    // 掉落物品
    if (PlayerInventory && LootTable.Num() > 0)
    {
        for (const FBMLootItem& LootItem : LootTable)
        {
            // 概率判定
            const float Roll = FMath::FRand();
            if (Roll > LootItem.Probability)
            {
                continue; // 未通过概率检查
            }

            // 确定掉落数量
            const int32 Quantity = FMath::RandRange(
                FMath::Max(0, LootItem.MinQuantity),
                FMath::Max(0, LootItem.MaxQuantity)
            );

            if (Quantity <= 0)
            {
                continue;
            }

            // 尝试添加物品
            if (PlayerInventory->AddItem(LootItem.ItemID, Quantity))
            {
                // 移除前缀
                FString ItemName = LootItem.ItemID.ToString();
                ItemName.RemoveFromStart(TEXT("Item_"));
                DroppedItems.Add(FString::Printf(TEXT("%s x%d"), *ItemName, Quantity));
            }
        }
    }

    // 构建通知消息
    FString NotificationMessage;
    bool bHasLoot = false;

    if (TotalCurrency > 0)
    {
        NotificationMessage += FString::Printf(TEXT("Currency: +%d"), TotalCurrency);
        bHasLoot = true;
    }

    if (TotalExp > 0.0f)
    {
        if (bHasLoot)
        {
            NotificationMessage += TEXT(" | ");
        }
        NotificationMessage += FString::Printf(TEXT("XP: +%.0f"), TotalExp);
        bHasLoot = true;
    }

    if (DroppedItems.Num() > 0)
    {
        if (bHasLoot)
        {
            NotificationMessage += TEXT(" | ");
        }
        NotificationMessage += TEXT("Items: ");
        for (int32 i = 0; i < DroppedItems.Num(); ++i)
        {
            NotificationMessage += DroppedItems[i];
            if (i < DroppedItems.Num() - 1)
            {
                NotificationMessage += TEXT(", ");
            }
        }
        bHasLoot = true;
    }

    // 通过事件总线发送通知
    if (bHasLoot)
    {
        if (UGameInstance* GI = GetGameInstance())
        {
            if (UBMEventBusSubsystem* EventBus = GI->GetSubsystem<UBMEventBusSubsystem>())
            {
                EventBus->EmitNotify(FText::FromString(NotificationMessage));
            }
        }
    }
}

bool ABMEnemyBase::IsInAttackRange() const
{
    APawn* T = CurrentTarget.Get();
    if (!T) return false;

    float Dist2D = FVector::Dist2D(T->GetActorLocation(), GetActorLocation());
	Dist2D += 5.0f; // 容差
    if (AttackRangeOverride >= 0.f)
    {
        return Dist2D <= AttackRangeOverride;
    }

    // 无 override：只要存在一个 attack spec 能覆盖当前距离即可
    for (const FBMEnemyAttackSpec& S : AttackSpecs)
    {
        if (!S.Anim) continue;
        if (Combat && !Combat->IsCooldownReady(S.Id)) continue;
        if (Dist2D >= S.MinRange && Dist2D <= S.MaxRange)
        {
            return true;
        }
    }
    return false;
}

bool ABMEnemyBase::CanStartAttack() const
{
    if (GetWorld() && GetWorld()->GetTimeSeconds() < NextAttackAllowedTime)
    {
        return false;
    }
    if (!HasValidTarget()) return false;

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        if (Move->IsFalling()) return false;
    }

    APawn* T = CurrentTarget.Get();
    if (!T) return false;

    const float Dist2D = FVector::Dist2D(T->GetActorLocation(), GetActorLocation());

    for (const FBMEnemyAttackSpec& S : AttackSpecs)
    {
        if (!S.Anim) continue;
        if (Dist2D < S.MinRange || Dist2D > S.MaxRange) continue;

        // 冷却过滤
        if (Combat && !Combat->IsCooldownReady(S.Id)) continue;
        return true;
    }

    return false;
}

void ABMEnemyBase::CommitAttackCooldown(float CooldownSeconds)
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    // 计算随机间隔
    const float RandomDev = FMath::FRandRange(-GlobalAttackIntervalDeviation, GlobalAttackIntervalDeviation);
    const float ActualInterval = FMath::Max(0.f, GlobalAttackInterval + RandomDev);

    NextAttackAllowedTime = Now + ActualInterval;
}

bool ABMEnemyBase::SelectRandomAttackForCurrentTarget(FBMEnemyAttackSpec& OutSpec) const
{
    APawn* T = CurrentTarget.Get();
    if (!T) return false;

    const float Dist2D = FVector::Dist2D(T->GetActorLocation(), GetActorLocation());

    // 过滤可用攻击并记录权重
    TArray<const FBMEnemyAttackSpec*> Candidates;
    TArray<float> Weights;
    float TotalW = 0.f;

    for (const FBMEnemyAttackSpec& S : AttackSpecs)
    {
        if (!S.Anim) continue;
        if (Dist2D < S.MinRange || Dist2D > S.MaxRange) continue;
        if (Combat && !Combat->IsCooldownReady(S.Id)) continue;
        
        const float W = FMath::Max(0.01f, S.Weight);
        Candidates.Add(&S);
        Weights.Add(W);
        TotalW += W;
    }

    if (Candidates.Num() == 0) return false;

    // 按权重随机选择
    float R = FMath::FRandRange(0.f, TotalW);
    for (int32 i = 0; i < Candidates.Num(); ++i)
    {
        R -= Weights[i];
        if (R <= 0.f)
        {
            OutSpec = *Candidates[i];
            return true;
        }
    }

    // 容错：返回最后一个
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

float ABMEnemyBase::PlayOnce(UAnimSequence* Seq, float PlayRate, float StartTime, float MaxPlayTime)
{
    if (!Seq || !GetMesh())
    {
        return 0.f;
    }

    CurrentLoopAnim = nullptr;

    // 确保是单节点模式
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

    // 用 SingleNodeInstance 控制起始时间/播放率
    UAnimSingleNodeInstance* Inst = GetMesh()->GetSingleNodeInstance();

    // 第一次没创建 Instance，先 PlayAnimation 一次
    if (!Inst)
    {
        GetMesh()->PlayAnimation(Seq, false);
        Inst = GetMesh()->GetSingleNodeInstance();
    }

    const float Len = Seq->GetPlayLength();
    const float SafePlayRate = (PlayRate > 0.f) ? PlayRate : 1.0f;

    const float ClampedStart = FMath::Clamp(StartTime, 0.f, Len);
    const float Remaining = FMath::Max(0.f, Len - ClampedStart);

    // MaxPlayTime <= 0 表示播完整剩余段，否则裁剪
    const float EffectivePlayTime = (MaxPlayTime > 0.f) ? FMath::Min(Remaining, MaxPlayTime) : Remaining;

    if (Inst)
    {
        Inst->SetAnimationAsset(Seq, /*bIsLooping=*/false);
        Inst->SetPosition(ClampedStart, /*bFireNotifies=*/true);
        Inst->SetPlayRate(SafePlayRate);
        Inst->SetPlaying(true);
    }
    else
    {
        // 若无法控制起始时间/裁剪
        GetMesh()->PlayAnimation(Seq, false);
        return Len / SafePlayRate;
    }

    // 返回时长
    return EffectivePlayTime / SafePlayRate;
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

float ABMEnemyBase::PlayDodgeOnce()
{
    return PlayOnce(AnimDodge, DodgePlayRate);
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

static void ApplySingleNodePlayRate(USkeletalMeshComponent* Mesh, float Rate)
{
    if (!Mesh) return;
    if (UAnimSingleNodeInstance* Inst = Mesh->GetSingleNodeInstance())
    {
        Inst->SetPlayRate(FMath::Max(0.01f, Rate));
    }
}

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
    // 先切 Death 状态，再做通用逻辑
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

bool ABMEnemyBase::TryEvadeIncomingHit(const FBMDamageInfo& InInfo)
{
    UBMStatsComponent* S = GetStats();
    if (S && S->IsDead()) return false;

    UBMStateMachineComponent* M = GetFSM();
    if (!M) return false;

    if (!Combat) return false;

    // 冷却检查
    if (!Combat->IsCooldownReady(DodgeCooldownKey))
    {
        return false;
    }

    // 概率判定
    const float P = FMath::Clamp(DodgeOnHitChance, 0.f, 1.f);
    if (FMath::FRand() > P)
    {
        return false;
    }

    // 触发闪避：本次命中不受伤害（直接返回 true）
    Combat->CommitCooldown(DodgeCooldownKey, DodgeCooldown);

    // 锁定闪避方向：向“远离攻击者”的方向后撤
    DodgeLockedDir = ComputeBackwardDodgeDirFromHit(InInfo);

    // 切 Dodge 状态
    M->ChangeStateByName(BMEnemyStateNames::Dodge);

    return true; // 不结算伤害
}

FVector ABMEnemyBase::ComputeBackwardDodgeDirFromHit(const FBMDamageInfo& InInfo) const
{
    FVector Dir = -GetActorForwardVector();

    if (AActor* Inst = InInfo.InstigatorActor.Get())
    {
        FVector Away = GetActorLocation() - Inst->GetActorLocation();
        Away.Z = 0.f;
        if (!Away.IsNearlyZero())
        {
            Dir = Away.GetSafeNormal();
        }
    }

    Dir.Z = 0.f;
    return Dir.IsNearlyZero() ? -GetActorForwardVector() : Dir.GetSafeNormal();
}

void ABMEnemyBase::InitFloatingHealthBar()
{
    if (!ShouldShowFloatingHealthBar())
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] InitFloatingHealthBar: Skipped (ShouldShowFloatingHealthBar=false)"), *GetName());
        return;
    }

    FloatingHealthBar = NewObject<UBMEnemyHealthBarComponent>(this, TEXT("FloatingHealthBar"));
    if (!FloatingHealthBar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] InitFloatingHealthBar: Failed to create component"), *GetName());
        return;
    }

    FloatingHealthBar->SetupAttachment(GetCapsuleComponent());
    FloatingHealthBar->RegisterComponent();

    const float CapsuleHalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 90.f;
    const float TotalOffset = CapsuleHalfHeight + FloatingHealthBarOffset;
    FloatingHealthBar->SetVerticalOffset(TotalOffset);

    FloatingHealthBar->ObserveCharacter(this);

    FloatingHealthBar->SetVisibility(bIsAlert, true);

    UE_LOG(LogTemp, Log, TEXT("[%s] InitFloatingHealthBar: Created at offset %.1f (CapsuleHalf=%.1f + Custom=%.1f), InitialVisible=%s"),
        *GetName(), TotalOffset, CapsuleHalfHeight, FloatingHealthBarOffset, bIsAlert ? TEXT("true") : TEXT("false"));
}

void ABMEnemyBase::LoadStatsFromDataTable()
{
    const FName EnemyID = GetEnemyDataID();
    if (EnemyID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] LoadStatsFromDataTable: GetEnemyDataID() returned None, using default stats"), *GetName());
        return;
    }

    UBMDataSubsystem* DataSys = GetGameInstance()->GetSubsystem<UBMDataSubsystem>();
    if (!DataSys)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] LoadStatsFromDataTable: BMDataSubsystem not found, using default stats"), *GetName());
        return;
    }

    const FBMEnemyData* Data = DataSys->GetEnemyData(EnemyID);
    if (!Data)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] LoadStatsFromDataTable: EnemyData not found for ID '%s', using default stats"), 
            *GetName(), *EnemyID.ToString());
        return;
    }

    // 应用数据到 Stats
    if (UBMStatsComponent* MyStatsComp = GetStats())
    {
        FBMStatBlock& MyStats = MyStatsComp->GetStatBlockMutable();
        MyStats.MaxHP = Data->MaxHP;
        MyStats.HP = Data->MaxHP;
        MyStats.Attack = Data->AttackPower;
        MyStats.Defense = Data->Defense;
        MyStats.MoveSpeed = Data->MoveSpeed;
    }

    // AI 参数
    AggroRange = Data->AggroRange;
    PatrolRadius = Data->PatrolRadius;
    PatrolSpeed = Data->PatrolSpeed;
    ChaseSpeed = Data->ChaseSpeed;
    
    // 战斗参数
    DodgeDistance = Data->DodgeDistance;
    DodgeOnHitChance = Data->DodgeOnHitChance;
    DodgeCooldown = Data->DodgeCooldown;
    DodgePlayRate = Data->DodgePlayRate;
    
    // Apply loot parameters
    CurrencyDropMin = Data->CurrencyDropMin;
    CurrencyDropMax = Data->CurrencyDropMax;
    ExpDropMin = Data->ExpDropMin;
    ExpDropMax = Data->ExpDropMax;

    // Load assets from DataTable
    LoadAssetsFromDataTable(Data);

    UE_LOG(LogTemp, Log, TEXT("[%s] LoadStatsFromDataTable: Loaded stats for '%s' - HP=%.0f, Attack=%.0f, Defense=%.0f"), 
        *GetName(), *EnemyID.ToString(), Data->MaxHP, Data->AttackPower, Data->Defense);
}

void ABMEnemyBase::LoadAssetsFromDataTable(const FBMEnemyData* Data)
{
    if (!Data)
    {
        return;
    }

    // Load Skeletal Mesh
    if (!Data->MeshPath.IsNull())
    {
        if (USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(Data->MeshPath.TryLoad()))
        {
            GetMesh()->SetSkeletalMesh(LoadedMesh);
            UE_LOG(LogTemp, Log, TEXT("[%s] Loaded Mesh from DataTable: %s"), *GetName(), *Data->MeshPath.ToString());
        }
    }

    // Load Animation Assets
    if (!Data->AnimIdlePath.IsNull())
    {
        AnimIdle = Cast<UAnimSequence>(Data->AnimIdlePath.TryLoad());
    }
    
    if (!Data->AnimWalkPath.IsNull())
    {
        AnimWalk = Cast<UAnimSequence>(Data->AnimWalkPath.TryLoad());
    }
    
    if (!Data->AnimRunPath.IsNull())
    {
        AnimRun = Cast<UAnimSequence>(Data->AnimRunPath.TryLoad());
    }
    
    if (!Data->AnimHitLightPath.IsNull())
    {
        AnimHitLight = Cast<UAnimSequence>(Data->AnimHitLightPath.TryLoad());
    }
    
    if (!Data->AnimHitHeavyPath.IsNull())
    {
        AnimHitHeavy = Cast<UAnimSequence>(Data->AnimHitHeavyPath.TryLoad());
    }
    
    if (!Data->AnimDeathPath.IsNull())
    {
        AnimDeath = Cast<UAnimSequence>(Data->AnimDeathPath.TryLoad());
    }
    
    if (!Data->AnimDodgePath.IsNull())
    {
        AnimDodge = Cast<UAnimSequence>(Data->AnimDodgePath.TryLoad());
    }

    UE_LOG(LogTemp, Log, TEXT("[%s] Loaded animations from DataTable"), *GetName());
}



