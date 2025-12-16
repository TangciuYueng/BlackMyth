#include "Character/Enemy/BMEnemyDummy.h"

#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Core/BMTypes.h"

ABMEnemyDummy::ABMEnemyDummy()
{
    // ===== 基类字段（类图字段）=====
    AggroRange = DummyAggroRange;
    PatrolRadius = DummyPatrolRadius;
    PatrolSpeed = DummyPatrolSpeed;
    ChaseSpeed = DummyChaseSpeed;

    // 网格偏移（按你项目习惯）
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    BuildHurtBoxes();
    BuildHitBoxes();

    // ===== 资产路径占位：你改这里即可 =====
    // 注意：这是“软引用”，不依赖蓝图；BeginPlay 里同步加载一次（Dummy 用于测试足够）
    MeshAsset = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Script/Engine.SkeletalMesh'/Game/Monster/Mesh/SK_Monster.SK_Monster'")));
    AnimIdleAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Monster/Animations/Demo/ThirdPersonIdle.ThirdPersonIdle'")));
    AnimWalkAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Monster/Animations/Demo/ThirdPersonWalk.ThirdPersonWalk'")));
    AnimRunAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Monster/Animations/Demo/ThirdPersonRun.ThirdPersonRun'")));
    AnimHitLightAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/HitReact_Front.HitReact_Front'")));
    AnimHitHeavyAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/HitReact_Front.HitReact_Front'")));
    AnimDeathAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Characters/Mannequins/Anims/Death/MM_Death_Front_01.MM_Death_Front_01'")));

    AttackLightAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.MM_Attack_01'")));
    AttackHeavy1Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02.MM_Attack_02'")));
    AttackHeavy2Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_03.MM_Attack_03'")));
}

void ABMEnemyDummy::BeginPlay()
{
    // 先加载资产并写入基类 Anim 指针，再让 Super::BeginPlay() 进入 FSM（避免 Idle 先播空动画）
    ApplyConfiguredAssets();
    // 组件/配置（HurtBox/HitBox/AttackSpecs）也在 Super 之前准备好，FSM 进入后直接可用
    BuildAttackSpecs();

    // 调试：启用 HitBox/HurtBox 可视化
    if (UBMHitBoxComponent* HB = GetHitBox()) HB->bDebugDraw = true;
    for (UBMHurtBoxComponent* HB : HurtBoxes)
    {
        if (!HB) continue;
        HB->bDebugDraw = true;
    }
    Super::BeginPlay();
}

void ABMEnemyDummy::ApplyConfiguredAssets()
{
    // SkeletalMesh
    if (!MeshAsset.IsNull())
    {
        if (USkeletalMesh* MyMesh = MeshAsset.LoadSynchronous())
        {
            GetMesh()->SetSkeletalMesh(MyMesh);
        }
    }

    // 下面这些是 ABMEnemyBase 里用于 FSM 播放的动画指针（巡逻Walk/追击Run/受击/死亡）
    AnimIdle = AnimIdleAsset.IsNull() ? nullptr : AnimIdleAsset.LoadSynchronous();
    AnimWalk = AnimWalkAsset.IsNull() ? nullptr : AnimWalkAsset.LoadSynchronous();
    AnimRun = AnimRunAsset.IsNull() ? nullptr : AnimRunAsset.LoadSynchronous();
    AnimHitLight = AnimHitLightAsset.IsNull() ? nullptr : AnimHitLightAsset.LoadSynchronous();
    AnimHitHeavy = AnimHitHeavyAsset.IsNull() ? nullptr : AnimHitHeavyAsset.LoadSynchronous();
    AnimDeath = AnimDeathAsset.IsNull() ? nullptr : AnimDeathAsset.LoadSynchronous();
}

void ABMEnemyDummy::BuildAttackSpecs()
{
    AttackSpecs.Reset();
    auto MakeWindowParams = [](float DamageMul, EBMHitReaction OverrideReaction)
    {
            FBMHitBoxActivationParams P;
            P.bResetHitRecords = true;                 // 每次进入攻击窗口都清命中记录（常用）
            P.DedupPolicy = EBMHitDedupPolicy::PerWindow;
            P.MaxHitsPerTarget = 1;
            P.DamageMultiplier = DamageMul;

            P.bOverrideReaction = (OverrideReaction != EBMHitReaction::None);
            P.OverrideReaction = OverrideReaction;
            return P;
    };

    // 轻攻击（可被打断）
    {
        FBMEnemyAttackSpec S;
        S.Anim = AttackLightAsset.IsNull() ? nullptr : AttackLightAsset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Light;
        S.Weight = 2.0f;
        S.MinRange = 0.f;  S.MaxRange = 180.f;
        S.Cooldown = 1.0f; S.PlayRate = 1.0f;

        S.bUninterruptible = false;
        S.InterruptChance = 0.65f;
        S.InterruptChanceOnHeavyHit = 1.0f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        // 关键：按 Name 指定要开的盒子（可多个）
        S.HitBoxNames = { TEXT("hand_r") };
        S.HitBoxParams = MakeWindowParams(/*DamageMul=*/1.0f, EBMHitReaction::Light);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // 重攻击1（霸体：不可打断）
    {
        FBMEnemyAttackSpec S;
        S.Anim = AttackHeavy1Asset.IsNull() ? nullptr : AttackHeavy1Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Heavy;
        S.Weight = 1.0f;
        S.MinRange = 0.f;  S.MaxRange = 220.f;
        S.Cooldown = 1.6f; S.PlayRate = 1.0f;

        S.bUninterruptible = true;
        S.InterruptChance = 0.0f;
        S.InterruptChanceOnHeavyHit = 0.0f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("hand_l") };
        S.HitBoxParams = MakeWindowParams(/*DamageMul=*/1.15f, EBMHitReaction::Heavy);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // 重攻击2（“重但可打断”：轻受击小概率打断，重受击大概率打断）
    {
        FBMEnemyAttackSpec S;
        S.Anim = AttackHeavy2Asset.IsNull() ? nullptr : AttackHeavy2Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Heavy;
        S.Weight = 1.0f;
        S.MinRange = 120.f; S.MaxRange = 260.f;
        S.Cooldown = 1.8f;  S.PlayRate = 1.0f;

        S.bUninterruptible = false;
        S.InterruptChance = 0.15f;
        S.InterruptChanceOnHeavyHit = 0.85f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("foot_r") };
        S.HitBoxParams = MakeWindowParams(/*DamageMul=*/1.35f, EBMHitReaction::Heavy);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // 伤害基值（交给 HitBoxComponent 再结合倍率/元素等）
    if (UBMHitBoxComponent* HB = GetHitBox())
    {
        HB->SetDamage(DummyBaseDamage);
    }
}

void ABMEnemyDummy::BuildHurtBoxes()
{
    // 说明：你的 UBMHurtBoxComponent 之前是“配置式组件”（AttachSocketOrBone/BoxExtent 等字段）
    // 这里沿用同样写法，避免与现有实现冲突。
    if (!HurtBody)
    {
        HurtBody = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Body"));
        HurtBody->AttachSocketOrBone = TEXT("spine_03");
        HurtBody->BoxExtent = FVector(16.f, 20.f, 30.f);
        HurtBody->DamageMultiplier = 1.0f;

        HurtBoxes.Add(HurtBody);
    }

    if (!HurtHead)
    {
        HurtHead = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Head"));
        HurtHead->AttachSocketOrBone = TEXT("head");
        HurtHead->BoxExtent = FVector(12.f, 12.f, 12.f);
        HurtHead->DamageMultiplier = 1.4f;

        HurtBoxes.Add(HurtHead);
    }
}

void ABMEnemyDummy::BuildHitBoxes()
{
    UBMHitBoxComponent* HB = GetHitBox();
    if (!HB) return;

    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("hand_r");
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

    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("hand_l");
        Def.Type = EBMHitBoxType::HeavyAttack;
        Def.AttachSocketOrBone = TEXT("hand_l");
        Def.BoxExtent = FVector(12.f, 12.f, 12.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.ElementType = EBMElementType::Physical;
        Def.DamageScale = 1.35f;
        Def.DefaultReaction = EBMHitReaction::Heavy;
        Def.KnockbackStrength = 140.f;

        HB->RegisterDefinition(Def);
    }

    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("foot_r");
        Def.Type = EBMHitBoxType::HeavyAttack;
        Def.AttachSocketOrBone = TEXT("foot_r");
        Def.BoxExtent = FVector(12.f, 12.f, 12.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.ElementType = EBMElementType::Physical;
        Def.DamageScale = 1.35f;
        Def.DefaultReaction = EBMHitReaction::Heavy;
        Def.KnockbackStrength = 140.f;

        HB->RegisterDefinition(Def);
    }


}
