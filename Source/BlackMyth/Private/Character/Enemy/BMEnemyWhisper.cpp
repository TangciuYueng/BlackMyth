#include "Character/Enemy/BMEnemyWhisper.h"

#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Core/BMTypes.h"

ABMEnemyWhisper::ABMEnemyWhisper()
{
    AggroRange = WhisperAggroRange;
    PatrolRadius = WhisperPatrolRadius;
    PatrolSpeed = WhisperPatrolSpeed;
    ChaseSpeed = WhisperChaseSpeed;

    DodgeDistance = WhisperDodgeDistance;
    DodgeOnHitChance = WhisperDodgeOnHitChance;
    DodgeCooldown = WhisperDodgeCooldown;
    DodgeCooldownKey = WhisperDodgeCooldownKey;
    DodgePlayRate = WhisperDodgePlayRate;

    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    BuildHurtBoxes();
    BuildHitBoxes();

    MeshAsset = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(
        TEXT("/Script/Engine.SkeletalMesh'/Game/Whisper/Base_Mesh/SK_Whisper.SK_Whisper'")));

    AnimIdleAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Idle1.Anim_Whisper_Idle1'")));
    AnimWalkAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Walk1.Anim_Whisper_Walk1'")));
    AnimRunAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Run.Anim_Whisper_Run'")));

    AnimHitLightAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Taking_Damage1.Anim_Whisper_Taking_Damage1'")));
    AnimHitHeavyAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Taking_Damage2.Anim_Whisper_Taking_Damage2'")));
    AnimDeathAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Death.Anim_Whisper_Death'")));

    AnimDodgeAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Q_Flip_Bwd.Q_Flip_Bwd'")));

    AttackLight1Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Attack1.Anim_Whisper_Attack1'")));
    AttackLight2Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Attack2.Anim_Whisper_Attack2'")));
    AttackHeavyAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Attack3.Anim_Whisper_Attack3'")));
}

void ABMEnemyWhisper::BeginPlay()
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

void ABMEnemyWhisper::ApplyConfiguredAssets()
{
    if (!MeshAsset.IsNull())
    {
        if (USkeletalMesh* MyMesh = MeshAsset.LoadSynchronous())
        {
            GetMesh()->SetSkeletalMesh(MyMesh);
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

void ABMEnemyWhisper::BuildHurtBoxes()
{
    if (!HurtBody)
    {
        HurtBody = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Body"));
        HurtBody->AttachSocketOrBone = TEXT("spine_03");
        HurtBody->BoxExtent = FVector(20.f, 25.f, 40.f);
        HurtBody->DamageMultiplier = 1.0f;
        HurtBoxes.Add(HurtBody);
    }

    if (!HurtAbdomen)
    {
        HurtAbdomen = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Abdomen"));
        HurtAbdomen->AttachSocketOrBone = TEXT("spine_01");
        HurtAbdomen->BoxExtent = FVector(20.f, 40.f, 25.f);
        HurtAbdomen->DamageMultiplier = 0.9f;
        HurtBoxes.Add(HurtAbdomen);
    }

    if (!HurtHead)
    {
        HurtHead = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Head"));
        HurtHead->AttachSocketOrBone = TEXT("head");
        HurtHead->BoxExtent = FVector(16.f, 16.f, 16.f);
        HurtHead->DamageMultiplier = 1.4f;
        HurtBoxes.Add(HurtHead);
    }
}

void ABMEnemyWhisper::BuildHitBoxes()
{
    UBMHitBoxComponent* HB = GetHitBox();
    if (!HB) return;

    // 右手
    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("whisper_hand_r");
        Def.Type = EBMHitBoxType::LightAttack;
        Def.AttachSocketOrBone = TEXT("hand_r");
        Def.BoxExtent = FVector(8.f, 8.f, 8.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.ElementType = EBMElementType::Physical;
        Def.DamageScale = 1.0f;
        Def.DefaultReaction = EBMHitReaction::Light;
        Def.KnockbackStrength = 90.f;

        HB->RegisterDefinition(Def);
    }

    // 左手
    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("whisper_hand_l");
        Def.Type = EBMHitBoxType::HeavyAttack;
        Def.AttachSocketOrBone = TEXT("hand_l");
        Def.BoxExtent = FVector(8.f, 8.f, 8.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.ElementType = EBMElementType::Physical;
        Def.DamageScale = 1.05f;
        Def.DefaultReaction = EBMHitReaction::Heavy;
        Def.KnockbackStrength = 95.f;

        HB->RegisterDefinition(Def);
    }

}

void ABMEnemyWhisper::BuildAttackSpecs()
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

    // 轻攻击1
    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Whisper_Light_01");
        S.Anim = AttackLight1Asset.IsNull() ? nullptr : AttackLight1Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Light;
        S.Weight = 2.0f;

        S.MinRange = 0.f;
        S.MaxRange = 80.f;
        S.Cooldown = 3.0f;
        S.PlayRate = 1.0f;

        S.bUninterruptible = false;
        S.InterruptChance = 0.65f;
        S.InterruptChanceOnHeavyHit = 1.0f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("whisper_hand_r") };
        S.HitBoxParams = MakeWindowParams(1.0f, EBMHitReaction::Light);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // 轻攻击2
    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Whisper_Light_02");
        S.Anim = AttackLight2Asset.IsNull() ? nullptr : AttackLight2Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Light;
        S.Weight = 2.0f;

        S.MinRange = 0.f;
        S.MaxRange = 80.f;
        S.Cooldown = 4.0f;
        S.PlayRate = 1.0f;

        S.bUninterruptible = false;
        S.InterruptChance = 0.55f;
        S.InterruptChanceOnHeavyHit = 0.95f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("whisper_hand_r") };
        S.HitBoxParams = MakeWindowParams(1.05f, EBMHitReaction::Light);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // 重攻击
    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Whisper_Heavy_01");
        S.Anim = AttackHeavyAsset.IsNull() ? nullptr : AttackHeavyAsset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Heavy;
        S.Weight = 1.0f;

        S.MinRange = 0.f;
        S.MaxRange = 80.f;
        S.Cooldown = 6.0f;
        S.PlayRate = 1.0f;

        S.bUninterruptible = false;
        S.InterruptChance = 0.10f;
        S.InterruptChanceOnHeavyHit = 0.3f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("whisper_hand_r"), TEXT("whisper_hand_l") };
        S.HitBoxParams = MakeWindowParams(1.25f, EBMHitReaction::Heavy);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // 伤害基值
    if (UBMHitBoxComponent* HB = GetHitBox())
    {
        HB->SetDamage(WhisperBaseDamage);
    }
}

float ABMEnemyWhisper::PlayDodgeOnce()
{
    // 与 Dummy 一致的裁剪方式
    return PlayOnce(AnimDodge, DodgePlayRate, 0.0, 0.7);
}
