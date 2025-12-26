#include "Character/Enemy/BMEnemyDemon.h"

#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Core/BMTypes.h"

ABMEnemyDemon::ABMEnemyDemon()
{

    AggroRange = DemonAggroRange;
    PatrolRadius = DemonPatrolRadius;
    PatrolSpeed = DemonPatrolSpeed;
    ChaseSpeed = DemonChaseSpeed;
    DodgeDistance = DemonDodgeDistance;
    DodgeOnHitChance = DemonDodgeOnHitChance;
    DodgeCooldown = DemonDodgeCooldown;
    DodgeCooldownKey = DemonDodgeCooldownKey;


    // 网格偏移
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    BuildHurtBoxes();
    BuildHitBoxes();

    AttackLightAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.MM_Attack_01'")));
    AttackHeavy1Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02.MM_Attack_02'")));
    AttackHeavy2Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_03.MM_Attack_03'")));
}

void ABMEnemyDemon::BeginPlay()
{
    // 优先从 DataTable 读取配置
    LoadStatsFromDataTable();
    
    // 先加载资产并写入基类 Anim 指针
    // ApplyConfiguredAssets();
    // 组件/配置（HurtBox/HitBox/AttackSpecs）
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

void ABMEnemyDemon::ApplyConfiguredAssets()
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
    AnimDodge = AnimDodgeAsset.IsNull() ? nullptr : AnimDodgeAsset.LoadSynchronous();
}

void ABMEnemyDemon::BuildAttackSpecs()
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
        S.Id = TEXT("Demon_Light_01");
        S.Anim = AttackLightAsset.IsNull() ? nullptr : AttackLightAsset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Light;
        S.Weight = 2.0f;
        S.MinRange = 0.f;  S.MaxRange = 100.f;
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
        S.Id = TEXT("Demon_Heavy_01");
        S.Anim = AttackHeavy1Asset.IsNull() ? nullptr : AttackHeavy1Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Heavy;
        S.Weight = 1.0f;
        S.MinRange = 0.f;  S.MaxRange = 100.f;
        S.Cooldown = 2.0f; S.PlayRate = 1.0f;

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
        S.Id = TEXT("Demon_Heavy_02");
        S.Anim = AttackHeavy2Asset.IsNull() ? nullptr : AttackHeavy2Asset.LoadSynchronous();
        S.AttackWeight = EBMEnemyAttackWeight::Heavy;
        S.Weight = 1.0f;
        S.MinRange = 100.f; S.MaxRange = 160.f;
        S.Cooldown = 3.0f;  S.PlayRate = 1.0f;

        S.bUninterruptible = false;
        S.InterruptChance = 0.15f;
        S.InterruptChanceOnHeavyHit = 0.85f;

        S.bStopPathFollowingOnEnter = true;
        S.bFaceTargetOnEnter = true;

        S.HitBoxNames = { TEXT("foot_r") };
        S.HitBoxParams = MakeWindowParams(/*DamageMul=*/1.35f, EBMHitReaction::Heavy);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // 伤害基值（已废弃，通过 DataTable 设置）
    // if (UBMHitBoxComponent* HB = GetHitBox())
    // {
    //     HB->SetDamage(DemonBaseDamage);
    // }
}

void ABMEnemyDemon::BuildHurtBoxes()
{

    if (!HurtBody)
    {
        HurtBody = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Body"));
        HurtBody->AttachSocketOrBone = TEXT("spine_03");
        HurtBody->BoxExtent = FVector(20.f, 25.f, 40.f);
        HurtBody->DamageMultiplier = 1.0f;

        HurtBoxes.Add(HurtBody);
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

void ABMEnemyDemon::BuildHitBoxes()
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

void ABMEnemyDemon::BuildLootTable()
{
    LootTable.Reset();


    {
        FBMLootItem Item;
        Item.ItemID = TEXT("Item_JiuZhuanJinDan");
        Item.ItemType = EBMItemType::Consumable;
        Item.Rarity = EBMItemRarity::Common;
        Item.Probability = 0.80f;
        Item.MinQuantity = 1;
        Item.MaxQuantity = 3;
        Item.Weight = 1.0f;

        LootTable.Add(Item);
    }


    {
        FBMLootItem Item;
        Item.ItemID = TEXT("Item_TaiYiZiJinDan");
        Item.ItemType = EBMItemType::Consumable;
        Item.Rarity = EBMItemRarity::Legendary;
        Item.Probability = 0.2f;
        Item.MinQuantity = 1;
        Item.MaxQuantity = 1;
        Item.Weight = 1.0f;

        LootTable.Add(Item);
    }

    UE_LOG(LogTemp, Log, TEXT("[%s] BuildLootTable: Configured %d loot items"),
        *GetName(), LootTable.Num());
}

float ABMEnemyDemon::PlayDodgeOnce()
{
    return PlayOnce(AnimDodge, DodgePlayRate, 0.0, 0.7);
}
