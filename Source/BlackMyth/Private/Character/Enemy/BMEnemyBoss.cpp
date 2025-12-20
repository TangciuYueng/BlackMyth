#include "Character/Enemy/BMEnemyBoss.h"

#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"

ABMEnemyBoss::ABMEnemyBoss()
{
    // ===== 基类可调参数（AI/移动/闪避）=====
    AggroRange = BossAggroRange;
    PatrolRadius = BossPatrolRadius;
    PatrolSpeed = BossPatrolSpeed;
    ChaseSpeed = BossChaseSpeed;

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
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Idle.Idle'")));
    AnimWalkAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Jog_Biped_Fwd.Jog_Biped_Fwd'")));
    AnimRunAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/TravelMode_Fwd_Start.TravelMode_Fwd_Start'")));

    AnimHitLightAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/HitReact_Front.HitReact_Front'")));
    AnimHitHeavyAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Stun_Start.Stun_Start'")));
    AnimDeathAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Death_A.Death_A'")));

    AnimDodgeAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/TravelMode_Bwd_Downhill.TravelMode_Bwd_Downhill'")));

    AttackLight1Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Attack_Biped_Melee_A.Attack_Biped_Melee_A'")));
    AttackLight2Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Attack_Biped_Melee_B.Attack_Biped_Melee_B'")));
    AttackHeavy1Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonRampage/Characters/Heroes/Rampage/Animations/Ability_RMB_Smash.Ability_RMB_Smash'")));
    
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

    // 调试可视化
    if (UBMHitBoxComponent* HB = GetHitBox()) HB->bDebugDraw = true;
    for (UBMHurtBoxComponent* HB : HurtBoxes)
    {
        if (HB) HB->bDebugDraw = true;
    }

    Super::BeginPlay();
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
        Def.BoxExtent = FVector(18.f, 18.f, 18.f);

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
        Def.BoxExtent = FVector(24.f, 24.f, 24.f);

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

    // 轻攻击：覆盖更近距离
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
        S.Id = TEXT("Boss_Heavy_02");
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
