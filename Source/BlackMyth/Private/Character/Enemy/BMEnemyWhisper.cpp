#include "Character/Enemy/BMEnemyWhisper.h"

#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Core/BMTypes.h"

/*
 * @brief Constructor of the ABMEnemyWhisper class
 */
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

    AttackLight1Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Attack1.Anim_Whisper_Attack1'")));
    AttackLight2Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Attack2.Anim_Whisper_Attack2'")));
    AttackHeavyAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(
        TEXT("/Script/Engine.AnimSequence'/Game/Whisper/Animations/Anim_Whisper_Attack3.Anim_Whisper_Attack3'")));
}

/*
 * @brief Begin play, it begins the play
 */
void ABMEnemyWhisper::BeginPlay()
{
    // ���ȴ� DataTable ��ȡ����
    LoadStatsFromDataTable();

    BuildAttackSpecs();
    BuildLootTable();

    // ���Կ��ӻ�
    //if (UBMHitBoxComponent* HB = GetHitBox()) HB->bDebugDraw = true;
    //for (UBMHurtBoxComponent* HB : HurtBoxes)
    //{
    //    if (HB) HB->bDebugDraw = true;
    //}

    Super::BeginPlay();
}

/*
 * @brief Apply configured assets, it applies the configured assets
 */
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

/*
 * @brief Build hurt boxes, it builds the hurt boxes
 */
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

/*
 * @brief Build hit boxes, it builds the hit boxes
 */
void ABMEnemyWhisper::BuildHitBoxes()
{
    UBMHitBoxComponent* HB = GetHitBox();
    if (!HB) return;

    // ����
    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("whisper_hand_r");
        Def.Type = EBMHitBoxType::LightAttack;
        Def.AttachSocketOrBone = TEXT("hand_r");
        Def.BoxExtent = FVector(8.f, 8.f, 8.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.DamageScale = 1.0f;
        Def.DefaultReaction = EBMHitReaction::Light;

        HB->RegisterDefinition(Def);
    }

    // ����
    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("whisper_hand_l");
        Def.Type = EBMHitBoxType::HeavyAttack;
        Def.AttachSocketOrBone = TEXT("hand_l");
        Def.BoxExtent = FVector(8.f, 8.f, 8.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.DamageScale = 1.05f;
        Def.DefaultReaction = EBMHitReaction::Heavy;

        HB->RegisterDefinition(Def);
    }

}

/*
 * @brief Build attack specs, it builds the attack specs
 */
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

    // �ṥ��1
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

    // �ṥ��2
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

    // �ع���
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
}

/*
 * @brief Build loot table, it builds the loot table
 */
void ABMEnemyWhisper::BuildLootTable()
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

/*
 * @brief Play dodge once, it plays the dodge once
 * @return The play time
 */
float ABMEnemyWhisper::PlayDodgeOnce()
{
    return PlayOnce(AnimDodge, DodgePlayRate, 0.0, 0.7);
}
