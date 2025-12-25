#include "Character/Enemy/BMEnemyBoss.h"

#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"
#include "Character/Components/BMStatsComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Engine/SkeletalMesh.h"
#include "BMGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Components/BMStatsComponent.h"

#include "Character/Enemy/States/BMEnemyBossState_PhaseChange.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Core/BMTypes.h"

ABMEnemyBoss::ABMEnemyBoss()
{
    // ===== 基类可调参数（AI/移动/闪避）=====
    AggroRange = BossAggroRange;
    PatrolRadius = BossPatrolRadius;
    PatrolSpeed = BossPatrolSpeed;
    ChaseSpeed = BossChaseSpeed;

	GlobalAttackInterval = BossGlobalAttackInterval;
	GlobalAttackIntervalDeviation = BossGlobalAttackIntervalDeviation;

    DodgeDistance = BossDodgeDistance;
    DodgeOnHitChance = BossDodgeOnHitChance;
    DodgeCooldown = BossDodgeCooldown;
    DodgeCooldownKey = BossDodgeCooldownKey;



    // ===== 体型/碰撞 =====
    ApplyBossBodyTuning();

    // ===== 先创建 HurtBox/HitBox 定义 =====
    BuildHurtBoxes();
    BuildHitBoxes();

    // ===== 资产路径占位）=====
    MeshAsset = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(
        TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonRampage/Characters/Heroes/Rampage/Meshes/Rampage.Rampage'")));

    AnimIdleAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Idle_Biped.Idle_Biped'")));
    AnimWalkAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Jog_Biped_Fwd.Jog_Biped_Fwd'")));
    AnimRunAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/TravelMode_Fwd_Start.TravelMode_Fwd_Start'")));

    AnimHitLightAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/HitReact_Front.HitReact_Front'")));
    AnimHitHeavyAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Stun_Start.Stun_Start'")));
    AnimDeathAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Death.Anim_Whisper_Death'")));

    AnimDodgeAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/TravelMode_Bwd_Downhill.TravelMode_Bwd_Downhill'")));

    AttackLight1Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Attack_Biped_Melee_A.Attack_Biped_Melee_A'")));
    AttackLight2Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Attack_Biped_Melee_B.Attack_Biped_Melee_B'")));
    AttackHeavy1Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Ability_RMB_Smash.Ability_RMB_Smash'")));

    AnimEnergizeAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Ability_Energize.Ability_Energize'")));
    AttackPhase2Heavy2Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Ability_GroundSmash_End.Ability_GroundSmash_End'")));
    AttackPhase2Light3Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Attack_Melee_C.Attack_Melee_C'")));

    
}
void ABMEnemyBoss::ApplyBossBodyTuning()
{
    // 1) Capsule：决定“物理体积/命中/寻路避障”
    if (UCapsuleComponent* Cap = GetCapsuleComponent())
    {
        Cap->SetCapsuleSize(BossCapsuleRadius, BossCapsuleHalfHeight);
    }

    // 2) Mesh：决定“视觉体型”
    if (USkeletalMeshComponent* TempMesh = GetMesh())
    {
        TempMesh->SetWorldScale3D(FVector(BossMeshScale));

        // 3) Mesh Z 偏移：按 scale 放大（避免脚浮空）
        const float Z = BaseMeshZOffset * BossMeshScale;
        TempMesh->SetRelativeLocation(FVector(0.f, 0.f, Z));

        TempMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    }
}

void ABMEnemyBoss::BeginPlay()
{
    ApplyConfiguredAssets();
    BuildAttackSpecs();

	// 注册二阶段转换状态
    if (UBMStateMachineComponent* Machine = GetFSM())
    {
        auto* SPhase = NewObject<UBMEnemyBossState_PhaseChange>(Machine);
        SPhase->Init(this);
        Machine->RegisterState(BMEnemyStateNames::PhaseChange, SPhase);
    }

    // 调试可视化
    if (UBMHitBoxComponent* HB = GetHitBox()) HB->bDebugDraw = true;
    for (UBMHurtBoxComponent* HB : HurtBoxes)
    {
        if (HB) HB->bDebugDraw = true;
    }

    Super::BeginPlay();

    // Bind to stats to track HP and death
    if (UBMStatsComponent* BossStats = GetStats())
    {
        BossStats->OnDeathNative.AddLambda([this](AActor* Killer)
        {
            if (UWorld* World = GetWorld())
            {
                if (UBMGameInstance* GI = Cast<UBMGameInstance>(World->GetGameInstance()))
                {
                    if (GetBossPhase() <= 1 && !bInPhaseTransition && !bReviveUsed)
                    {
                        // Stop current boss1 immediately, then delay-play boss2
                        GI->StopLevelMusic();
                        const float Delay = Phase1To2MusicDelaySeconds;
                        FTimerHandle Temp;
                        World->GetTimerManager().SetTimer(Temp, [this]()
                        {
                            if (UWorld* W = GetWorld())
                            {
                                if (UBMGameInstance* G = Cast<UBMGameInstance>(W->GetGameInstance()))
                                {
                                    G->PlayMusic(W, TEXT("/Game/Audio/boss2.boss2"), /*bLoop=*/true);
                                }
                            }
                        }, Delay, /*bLoop=*/false);
                    }
                    else
                    {
                        // If boss is in phase2 or this is a second death, stop boss2 music and mark defeated
                        if (GetBossPhase() >= 2)
                        {
                            GI->bIsBossPhase2Defeated = true;
                            GI->StopLevelMusic();
                            UE_LOG(LogTemp, Log, TEXT("BeginPlay death-callback: Boss phase2 defeated, stopped boss2 music."));
                        }
                        else
                        {
                            UE_LOG(LogTemp, Verbose, TEXT("BeginPlay death-callback: entering second-death branch (handled here).") );
                        }
                    }
                    UE_LOG(LogTemp, Warning, TEXT("Death handled: Phase %d, InTransition: %d, ReviveUsed: %d"), GetBossPhase(), bInPhaseTransition, bReviveUsed);
                }
            }
        });
    }
}

void ABMEnemyBoss::ApplyConfiguredAssets()
{
    if (!MeshAsset.IsNull())
    {
        if (USkeletalMesh* M = MeshAsset.LoadSynchronous())
        {
            GetMesh()->SetSkeletalMesh(M);
        }
    }

    AnimIdle = AnimIdleAsset.IsNull() ? nullptr : AnimIdleAsset.LoadSynchronous();
    AnimWalk = AnimWalkAsset.IsNull() ? nullptr : AnimWalkAsset.LoadSynchronous();
    AnimRun = AnimRunAsset.IsNull() ? nullptr : AnimRunAsset.LoadSynchronous();

    AnimHitLight = AnimHitLightAsset.IsNull() ? nullptr : AnimHitLightAsset.LoadSynchronous();
    AnimHitHeavy = AnimHitHeavyAsset.IsNull() ? nullptr : AnimHitHeavyAsset.LoadSynchronous();
    AnimDeath = AnimDeathAsset.IsNull() ? nullptr : AnimDeathAsset.LoadSynchronous();

    AnimDodge = AnimDodgeAsset.IsNull() ? nullptr : AnimDodgeAsset.LoadSynchronous();

    AnimEnergize = AnimEnergizeAsset.IsNull() ? nullptr : AnimEnergizeAsset.LoadSynchronous();
}

void ABMEnemyBoss::BuildHurtBoxes()
{
    if (!HurtBody)
    {
        HurtBody = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Body"));
        HurtBody->AttachSocketOrBone = TEXT("spine_03");
        HurtBody->BoxExtent = FVector(34.f, 40.f, 60.f);     
        HurtBody->DamageMultiplier = 1.0f;
        HurtBoxes.Add(HurtBody);
    }

    if (!HurtAbdomen)
    {
        HurtAbdomen = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Abdomen"));
        HurtAbdomen->AttachSocketOrBone = TEXT("spine_01");
        HurtAbdomen->BoxExtent = FVector(30.f, 36.f, 40.f);     
        HurtAbdomen->DamageMultiplier = 0.9f;                  
        HurtBoxes.Add(HurtAbdomen);
    }

    if (!HurtHead)
    {
        HurtHead = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Head"));
        HurtHead->AttachSocketOrBone = TEXT("head");
        HurtHead->BoxExtent = FVector(22.f, 22.f, 22.f);     
        HurtHead->DamageMultiplier = 1.25f;                  
        HurtBoxes.Add(HurtHead);
    }
}

void ABMEnemyBoss::BuildHitBoxes()
{
    UBMHitBoxComponent* HB = GetHitBox();
    if (!HB) return;

    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("boss_hand_r_light");
        Def.Type = EBMHitBoxType::LightAttack;
        Def.AttachSocketOrBone = TEXT("hand_r");
        Def.BoxExtent = FVector(45.f, 45.f, 45.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.ElementType = EBMElementType::Physical;
        Def.DamageScale = 1.0f;
        Def.DefaultReaction = EBMHitReaction::Light;
        Def.KnockbackStrength = 120.f;

        HB->RegisterDefinition(Def);
    }

    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("boss_hand_r_heavy");
        Def.Type = EBMHitBoxType::HeavyAttack;
        Def.AttachSocketOrBone = TEXT("hand_r");
        Def.BoxExtent = FVector(50.f, 100.f, 18.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.ElementType = EBMElementType::Physical;
        Def.DamageScale = 1.0f;
        Def.DefaultReaction = EBMHitReaction::Heavy;
        Def.KnockbackStrength = 120.f;

        HB->RegisterDefinition(Def);
    }

    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("boss_hand_l_light");
        Def.Type = EBMHitBoxType::LightAttack;
        Def.AttachSocketOrBone = TEXT("hand_l");
        Def.BoxExtent = FVector(45.f, 45.f, 45.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.ElementType = EBMElementType::Physical;
        Def.DamageScale = 1.10f;
        Def.DefaultReaction = EBMHitReaction::Light;
        Def.KnockbackStrength = 180.f;

        HB->RegisterDefinition(Def);
    }

    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("boss_hand_l_heavy");
        Def.Type = EBMHitBoxType::HeavyAttack;
        Def.AttachSocketOrBone = TEXT("hand_l");
        Def.BoxExtent = FVector(50.f, 100.f, 18.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.ElementType = EBMElementType::Physical;
        Def.DamageScale = 1.0f;
        Def.DefaultReaction = EBMHitReaction::Heavy;
        Def.KnockbackStrength = 120.f;

        HB->RegisterDefinition(Def);
    }

}

void ABMEnemyBoss::BuildAttackSpecs()
{
    AttackSpecs.Reset();

    auto MakeWindowParams = [](float DamageMul, EBMHitReaction OverrideReaction)
        {
            FBMHitBoxActivationParams P;
            P.bResetHitRecords = true;
            P.DedupPolicy = EBMHitDedupPolicy::PerWindow;
            P.MaxHitsPerTarget = 1;

            P.DamageMultiplier = DamageMul;
            P.bOverrideReaction = (OverrideReaction != EBMHitReaction::None);
            P.OverrideReaction = OverrideReaction;
            return P;
        };

    // 轻攻击
    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Boss_Light_01");
        S.Anim = AttackLight1Asset.IsNull() ? nullptr : AttackLight1Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Light;
        S.Weight = 3.0f;

        S.MinRange = 0.f;
        S.MaxRange = 160.f;
        S.Cooldown = 1.4f;
        S.PlayRate = 1.0f;

        S.bUninterruptible = false;
        S.InterruptChance = 0.35f;
        S.InterruptChanceOnHeavyHit = 0.85f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("boss_hand_r_light") };
        S.HitBoxParams = MakeWindowParams(1.0f, EBMHitReaction::Light);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // 重攻击1：霸体
    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Boss_Light_02");
        S.Anim = AttackLight2Asset.IsNull() ? nullptr : AttackLight2Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Light;
        S.Weight = 3.0f;

        S.MinRange = 0.f;
        S.MaxRange = 200.f;
        S.Cooldown = 2.6f;
        S.PlayRate = 0.95f;

        S.bUninterruptible = true;
        S.InterruptChance = 0.4f;
        S.InterruptChanceOnHeavyHit = 0.8f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("boss_hand_l_light") };
        S.HitBoxParams = MakeWindowParams(1.0f, EBMHitReaction::Heavy);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // 重攻击2：脚踢/砸地，远一点
    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Boss_Heavy_01");
        S.Anim = AttackHeavy1Asset.IsNull() ? nullptr : AttackHeavy1Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Heavy;
        S.Weight = 0.9f;

        S.MinRange = 0.f;
        S.MaxRange = 200.f;
        S.Cooldown = 3.3f;
        S.PlayRate = 0.9f;

        S.bUninterruptible = false;
        S.InterruptChance = 0.10f;
        S.InterruptChanceOnHeavyHit = 0.3f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("boss_hand_l_heavy"), TEXT("boss_hand_r_heavy") };
        S.HitBoxParams = MakeWindowParams(1.40f, EBMHitReaction::Heavy);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // 基础伤害
    if (UBMHitBoxComponent* HB = GetHitBox())
    {
        HB->SetDamage(BossBaseDamage);
    }
}

float ABMEnemyBoss::PlayDodgeOnce()
{
    // 你 Dummy 用的是 PlayOnce(AnimDodge, DodgePlayRate, 0, 0.7)
    return PlayOnce(AnimDodge, DodgePlayRate, 0.0, 0.75f);
}

float ABMEnemyBoss::PlayEnergizeOnce()
{
    return PlayOnce(AnimEnergize, 0.7f);
}

void ABMEnemyBoss::HandleDeath(const FBMDamageInfo& LastHitInfo)
{
    LastDamageInfo = LastHitInfo;

    // 第一次死亡
    if (!bReviveUsed)
    {
        bReviveUsed = true;

        // 进过渡状态
        if (UBMStateMachineComponent* Machine = GetFSM())
        {
            Machine->ChangeStateByName(BMEnemyStateNames::PhaseChange);
        }
        return;
    }

    // 第二次死亡
    // Mark boss-phase-2 defeated and trigger end-video only on the true second death
    if (UWorld* World = GetWorld())
    {
        if (UBMGameInstance* GI = Cast<UBMGameInstance>(World->GetGameInstance()))
        {
            GI->bIsBossPhase2Defeated = true;
            // Play end video here so it only fires on the second death
            GI->PlayEndVideo();
        }
    }

    Super::HandleDeath(LastHitInfo);
}

bool ABMEnemyBoss::CanBeDamagedBy(const FBMDamageInfo& Info) const
{
    if (bInPhaseTransition)
    {
        return false;
    }
    return Super::CanBeDamagedBy(Info);
}

void ABMEnemyBoss::EnterPhase2()
{
    if (bIsPhase2) return;
    bIsPhase2 = true;
    BossPhase = 2;

    ApplyPhase2Tuning();
    AddPhase2AttackSpecs();

    //// 二阶段回到 idle 动画
    //PlayIdleLoop();
}

void ABMEnemyBoss::ApplyPhase2Tuning()
{
    // 回满 + 提高最大血
    if (UBMStatsComponent* S = GetStats())
    {
        // Reset any boss-related music state in case boss was previously defeated and revived
        if (UWorld* World = GetWorld())
        {
            if (UBMGameInstance* GI = Cast<UBMGameInstance>(World->GetGameInstance()))
            {
                GI->ResetBossMusicState();
            }
        }

        S->ReviveToFull(Phase2MaxHP);
    }

    // 提高基础伤害
    if (UBMHitBoxComponent* HB = GetHitBox())
    {
        HB->SetDamage(Phase2BaseDamage);
    }

}

void ABMEnemyBoss::AddPhase2AttackSpecs()
{
    auto MakeWindowParams = [](float DamageMul, EBMHitReaction OverrideReaction)
        {
            FBMHitBoxActivationParams P;
            P.bResetHitRecords = true;
            P.DedupPolicy = EBMHitDedupPolicy::PerWindow;
            P.MaxHitsPerTarget = 1;
            P.DamageMultiplier = DamageMul;
            P.bOverrideReaction = (OverrideReaction != EBMHitReaction::None);
            P.OverrideReaction = OverrideReaction;
            return P;
        };

    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Boss_Ph2_Heavy_02");
        S.Anim = AttackPhase2Heavy2Asset.IsNull() ? nullptr : AttackPhase2Heavy2Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Heavy;
        S.Weight = 1.2f;

        S.MinRange = 0.f;
        S.MaxRange = 200.f;
        S.Cooldown = 3.8f;
        S.PlayRate = 0.7f;

        S.bUninterruptible = true;
        S.InterruptChance = 0.f;
        S.InterruptChanceOnHeavyHit = 0.f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("boss_hand_l_heavy"), TEXT("boss_hand_r_heavy") };
        S.HitBoxParams = MakeWindowParams(1.8f, EBMHitReaction::Heavy);

        if (S.Anim) AttackSpecs.Add(S);
    }

    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Boss_Ph2_Light_03");
        S.Anim = AttackPhase2Light3Asset.IsNull() ? nullptr : AttackPhase2Light3Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Light;
        S.Weight = 2.5f;

        S.MinRange = 0.f;
        S.MaxRange = 200.f;
        S.Cooldown = 1.2f;
        S.PlayRate = 1.0f;

        S.bUninterruptible = false;
        S.InterruptChance = 0.2f;
        S.InterruptChanceOnHeavyHit = 0.6f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("boss_hand_l_light") };
        S.HitBoxParams = MakeWindowParams(1.2f, EBMHitReaction::Light);

        if (S.Anim) AttackSpecs.Add(S);
    }
}

float ABMEnemyBoss::PlayDeathReverseOnce(float ReversePlayRate, float ReverseMaxTime)
{
    UAnimSequence* Seq = AnimDeath;
    if (!Seq || !GetMesh()) return 0.f;

    ReversePlayRate = FMath::Max(0.01f, ReversePlayRate);

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

    UAnimSingleNodeInstance* Inst = GetMesh()->GetSingleNodeInstance();
    if (!Inst)
    {
        // 先触发一次，确保 SingleNodeInstance 创建
        GetMesh()->PlayAnimation(Seq, false);
        Inst = GetMesh()->GetSingleNodeInstance();
    }
    if (!Inst) return Seq->GetPlayLength() / ReversePlayRate;

    const float Len = Seq->GetPlayLength();

    // 倒放区间
    const float Effective = (ReverseMaxTime > 0.f) ? FMath::Min(Len, ReverseMaxTime) : Len;

    // 从动画末尾开始倒放
    const float StartPos = Len;
    Inst->SetAnimationAsset(Seq, /*bIsLooping=*/false);
    Inst->SetPosition(StartPos, /*bFireNotifies=*/true);

    // 负播放率
    Inst->SetPlayRate(-ReversePlayRate);
    Inst->SetPlaying(true);

    return Effective / ReversePlayRate;
}
void ABMEnemyBoss::SetAlertState(bool bAlert)
{
    const bool bPrevAlert = IsAlerted();
    ABMEnemyBase::SetAlertState(bAlert);
    if (!bPrevAlert && bAlert && !bBossAlertMusicStarted)
    {
        if (UWorld* World = GetWorld())
        {
            if (UBMGameInstance* GI = Cast<UBMGameInstance>(World->GetGameInstance()))
            {
                GI->PlayMusic(World, TEXT("/Game/Audio/boss1.boss1"), /*bLoop=*/true);
                bBossAlertMusicStarted = true;
            }
        }
    }
}
