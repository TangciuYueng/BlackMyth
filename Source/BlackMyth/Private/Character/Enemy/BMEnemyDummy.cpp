#include "Character/Enemy/BMEnemyDummy.h"

#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Core/BMTypes.h"

/*
 * @brief Constructor of the ABMEnemyDummy class
 */
ABMEnemyDummy::ABMEnemyDummy()
{
    
    AggroRange = DummyAggroRange;
    PatrolRadius = DummyPatrolRadius;
    PatrolSpeed = DummyPatrolSpeed;
    ChaseSpeed = DummyChaseSpeed;
	DodgeDistance = DummyDodgeDistance;
	DodgeOnHitChance = DummyDodgeOnHitChance;
	DodgeCooldown = DummyDodgeCooldown;
	DodgeCooldownKey = DummyDodgeCooldownKey;
	CurrencyDropMin = DummyCurrencyDropMin;
	CurrencyDropMax = DummyCurrencyDropMax;
	ExpDropMax = DummyExpDropMax;
	ExpDropMin = DummyExpDropMin;


    // ����ƫ��
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    BuildHurtBoxes();
    BuildHitBoxes();
    

    AttackLightAsset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.MM_Attack_01'")));
    AttackHeavy1Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02.MM_Attack_02'")));
    AttackHeavy2Asset = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(TEXT("/Script/Engine.AnimSequence'/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_03.MM_Attack_03'")));
}

/*
 * @brief Begin play, it begins the play
 */
void ABMEnemyDummy::BeginPlay()
{
    // ���ȴ� DataTable ��ȡ����
    LoadStatsFromDataTable();
    
    BuildAttackSpecs();
    BuildLootTable();
    // ���ԣ����� HitBox/HurtBox ���ӻ�
    //if (UBMHitBoxComponent* HB = GetHitBox()) HB->bDebugDraw = true;
    //for (UBMHurtBoxComponent* HB : HurtBoxes)
    //{
    //    if (!HB) continue;
    //    HB->bDebugDraw = true;
    //}
    Super::BeginPlay();
}

/*
 * @brief Apply configured assets, it applies the configured assets
 */
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

    // ������Щ�� ABMEnemyBase ������ FSM ���ŵĶ���ָ�루Ѳ��Walk/׷��Run/�ܻ�/������
    AnimIdle = AnimIdleAsset.IsNull() ? nullptr : AnimIdleAsset.LoadSynchronous();
    AnimWalk = AnimWalkAsset.IsNull() ? nullptr : AnimWalkAsset.LoadSynchronous();
    AnimRun = AnimRunAsset.IsNull() ? nullptr : AnimRunAsset.LoadSynchronous();
    AnimHitLight = AnimHitLightAsset.IsNull() ? nullptr : AnimHitLightAsset.LoadSynchronous();
    AnimHitHeavy = AnimHitHeavyAsset.IsNull() ? nullptr : AnimHitHeavyAsset.LoadSynchronous();
    AnimDeath = AnimDeathAsset.IsNull() ? nullptr : AnimDeathAsset.LoadSynchronous();
	AnimDodge = AnimDodgeAsset.IsNull() ? nullptr : AnimDodgeAsset.LoadSynchronous();
}

/*
 * @brief Build attack specs, it builds the attack specs
 */
void ABMEnemyDummy::BuildAttackSpecs()
{
    AttackSpecs.Reset();
    auto MakeWindowParams = [](float DamageMul, EBMHitReaction OverrideReaction)
    {
            FBMHitBoxActivationParams P;
            P.bResetHitRecords = true;                 // ÿ�ν��빥�����ڶ������м�¼
            P.DedupPolicy = EBMHitDedupPolicy::PerWindow;
            P.MaxHitsPerTarget = 1;
            P.DamageMultiplier = DamageMul;

            P.bOverrideReaction = (OverrideReaction != EBMHitReaction::None);
            P.OverrideReaction = OverrideReaction;
            return P;
    };

    // �ṥ��
    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Dummy_Light_01");
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

        S.HitBoxNames = { TEXT("hand_r") };
        S.HitBoxParams = MakeWindowParams(/*DamageMul=*/1.0f, EBMHitReaction::Light);

        if (S.Anim) AttackSpecs.Add(S);
    }

    // �ع���1
    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Dummy_Heavy_01");
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

    // �ع���2
    {
        FBMEnemyAttackSpec S;
        S.Id = TEXT("Dummy_Heavy_02");
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

}

/*
 * @brief Build hurt boxes, it builds the hurt boxes
 */
void ABMEnemyDummy::BuildHurtBoxes()
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

/*
 * @brief Build hit boxes, it builds the hit boxes
 */
void ABMEnemyDummy::BuildHitBoxes()
{
    UBMHitBoxComponent* HB = GetHitBox();
    if (!HB) return;

    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("hand_r");
        Def.Type = EBMHitBoxType::LightAttack;
        Def.AttachSocketOrBone = TEXT("hand_r");
        Def.BoxExtent = FVector(16.f, 16.f, 16.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.DamageScale = 1.0f;
        Def.DefaultReaction = EBMHitReaction::Light;

        HB->RegisterDefinition(Def);
    }

    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("hand_l");
        Def.Type = EBMHitBoxType::HeavyAttack;
        Def.AttachSocketOrBone = TEXT("hand_l");
        Def.BoxExtent = FVector(20.f, 20.f, 20.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.DamageScale = 1.35f;
        Def.DefaultReaction = EBMHitReaction::Heavy;

        HB->RegisterDefinition(Def);
    }

    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("foot_r");
        Def.Type = EBMHitBoxType::HeavyAttack;
        Def.AttachSocketOrBone = TEXT("foot_r");
        Def.BoxExtent = FVector(16.f, 16.f, 16.f);

        Def.DamageType = EBMDamageType::Melee;
        Def.DamageScale = 1.35f;
        Def.DefaultReaction = EBMHitReaction::Heavy;

        HB->RegisterDefinition(Def);
    }


}

/*
 * @brief Build loot table, it builds the loot table
 */
void ABMEnemyDummy::BuildLootTable()
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
float ABMEnemyDummy::PlayDodgeOnce()
{
    return PlayOnce(AnimDodge, DodgePlayRate, 0.0, 0.7);
}
