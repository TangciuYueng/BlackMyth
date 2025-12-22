#include "Character/Enemy/BMEnemyBase.h"

#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"
#include "Character/Components/BMHealthBarComponent.h"
#include "Character/Components/BMStatsComponent.h"

#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/BMDataSubsystem.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABMEnemyBase::ABMEnemyBase()
{
    CharacterType = EBMCharacterType::Enemy;
    Team = EBMTeam::Enemy;

    PrimaryActorTick.bCanEverTick = true;

    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = 0.f;
        MoveComp->bOrientRotationToMovement = false;
    }

    InitializeHurtBoxes();
    InitializeHitBoxes();

    HealthBarComponent = CreateDefaultSubobject<UBMEnemyHealthBarComponent>(TEXT("HealthBar"));
    if (HealthBarComponent)
    {
        HealthBarComponent->SetupAttachment(GetMesh());
        HealthBarComponent->SetRelativeLocation(FVector::ZeroVector);
        HealthBarComponent->SetVerticalOffset(120.f);
    }
}

void ABMEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    LoadEnemyDataAssets();
    CachePlayerPawn();
    PlayIdleLoop();

    // 调试：启用 HitBox/HurtBox 可视化
    if (UBMHitBoxComponent* HB = GetHitBox()) HB->bDebugDraw = true;
    for (UBMHurtBoxComponent* HB : HurtBoxes)
    {
        if (!HB) continue;
        HB->bDebugDraw = true;
    }
}

void ABMEnemyBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdatePerception(DeltaSeconds);

    if (bDrawAggroRange)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), AggroRange, 24, FColor::Red, false, 0.f, 0, 1.5f);
    }
}

void ABMEnemyBase::InitializeHurtBoxes()
{
    // 大部分近战小怪只需要头部 + 身体两个 HurtBox
    UBMHurtBoxComponent* Body = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Body"));
    if (Body)
    {
        Body->AttachSocketOrBone = TEXT("spine_03");
        Body->BoxExtent = FVector(16.f, 20.f, 30.f);
        Body->DamageMultiplier = 1.0f;
    }

    UBMHurtBoxComponent* Head = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Head"));
    if (Head)
    {
        Head->AttachSocketOrBone = TEXT("head");
        Head->BoxExtent = FVector(12.f, 12.f, 12.f);
        Head->DamageMultiplier = 1.4f;
    }
}

void ABMEnemyBase::InitializeHitBoxes()
{
    if (UBMHitBoxComponent* HB = GetHitBox())
    {
        FBMHitBoxDefinition Claw;
        Claw.Name = TEXT("Claw");
        Claw.Type = EBMHitBoxType::LightAttack;
        Claw.AttachSocketOrBone = TEXT("weapon_r");
        Claw.BoxExtent = FVector(8.f, 8.f, 8.f);
        Claw.RelativeTransform = FTransform(FRotator::ZeroRotator, FVector(0.f, 50.f, 0.f));
        Claw.DamageType = EBMDamageType::Melee;
        Claw.ElementType = EBMElementType::Physical;
        Claw.DamageScale = 1.0f;
        Claw.DefaultReaction = EBMHitReaction::Light;
        Claw.KnockbackStrength = 90.f;

        HB->RegisterDefinition(Claw);
        HB->SetDamage(BaseDamage);
    }
}

void ABMEnemyBase::PlayIdleLoop()
{
    if (!AnimIdle || !GetMesh())
    {
        return;
    }

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    GetMesh()->PlayAnimation(AnimIdle, true);
}

bool ABMEnemyBase::DetectPlayer() const
{
    if (AggroRange <= 0.f)
    {
        return false;
    }

    APawn* PlayerPawn = CachedPlayer.Get();
    if (!PlayerPawn)
    {
        if (UWorld* World = GetWorld())
        {
            PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
            const_cast<ABMEnemyBase*>(this)->CachedPlayer = PlayerPawn;
        }
    }

    if (!PlayerPawn)
    {
        return false;
    }

    const float DistSq = FVector::DistSquared(PlayerPawn->GetActorLocation(), GetActorLocation());
    return DistSq <= FMath::Square(AggroRange);
}

void ABMEnemyBase::DropLoot()
{
    if (LootTable.Num() == 0)
    {
        return;
    }

    for (const FBMLootItem& Item : LootTable)
    {
        if (Item.ItemID.IsNone())
        {
            continue;
        }

        const float Chance = FMath::Clamp(Item.Probability, 0.f, 1.f);
        if (FMath::FRand() > Chance)
        {
            continue;
        }

        const int32 Quantity = (Item.MinQuantity <= Item.MaxQuantity)
            ? FMath::RandRange(Item.MinQuantity, Item.MaxQuantity)
            : Item.MinQuantity;

        UE_LOG(LogBMCharacter, Log, TEXT("[%s] Dropped loot: %s x%d"),
            *GetName(), *Item.ItemID.ToString(), Quantity);
    }
}

void ABMEnemyBase::SetAlertState(bool bAlert)
{
    if (bIsAlert == bAlert)
    {
        return;
    }

    bIsAlert = bAlert;
    UE_LOG(LogBMCharacter, Log, TEXT("[%s] Alert state -> %s"), *GetName(), bIsAlert ? TEXT("Alert") : TEXT("Calm"));
}

void ABMEnemyBase::HandleDamageTaken(const FBMDamageInfo& FinalInfo)
{
    Super::HandleDamageTaken(FinalInfo);
    SetAlertState(true);
}

void ABMEnemyBase::HandleDeath(const FBMDamageInfo& LastHitInfo)
{
    Super::HandleDeath(LastHitInfo);

    DropLoot();

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->Stop();
    }

    SetLifeSpan(1.f);
}

bool ABMEnemyBase::CanBeDamagedBy(const FBMDamageInfo& Info) const
{
    if (!Super::CanBeDamagedBy(Info))
    {
        return false;
    }

    if (const ABMCharacterBase* InstigatorCharacter = Cast<ABMCharacterBase>(Info.InstigatorActor.Get()))
    {
        if (InstigatorCharacter->Team == Team)
        {
            return false;
        }
    }

    return true;
}

void ABMEnemyBase::UpdatePerception(float DeltaSeconds)
{
    (void)DeltaSeconds;

    const bool bPlayerDetected = DetectPlayer();
    if (bPlayerDetected != bIsAlert)
    {
        SetAlertState(bPlayerDetected);
    }
}

void ABMEnemyBase::LoadEnemyDataAssets()
{
    const FName DataId = ResolveEnemyDataId();

    if (!DataId.IsNone())
    {
        if (UWorld* World = GetWorld())
        {
            if (UGameInstance* GameInstance = World->GetGameInstance())
            {
                if (const UBMDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UBMDataSubsystem>())
                {
                    if (const FBMEnemyData* EnemyData = DataSubsystem->GetEnemyData(DataId))
                    {
                        if (!EnemyData->MeshPath.IsNull())
                        {
                            if (USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(EnemyData->MeshPath.TryLoad()))
                            {
                                EnemyMeshAsset = LoadedMesh;
                            }
                            else
                            {
                                UE_LOG(LogBMCharacter, Warning, TEXT("[%s] Failed to load enemy mesh asset: %s"),
                                    *GetName(), *EnemyData->MeshPath.ToString());
                            }
                        }

                        if (!EnemyData->AnimPath.IsNull())
                        {
                            if (UAnimSequence* LoadedIdle = Cast<UAnimSequence>(EnemyData->AnimPath.TryLoad()))
                            {
                                AnimIdle = LoadedIdle;
                            }
                            else
                            {
                                UE_LOG(LogBMCharacter, Warning, TEXT("[%s] Failed to load idle animation asset: %s"),
                                    *GetName(), *EnemyData->AnimPath.ToString());
                            }
                        }
                    }
                    else
                    {
                        UE_LOG(LogBMCharacter, Warning, TEXT("[%s] Enemy data row '%s' not found"), *GetName(), *DataId.ToString());
                    }
                }
                else
                {
                    UE_LOG(LogBMCharacter, Warning, TEXT("[%s] BMDataSubsystem unavailable, cannot load enemy data"), *GetName());
                }
            }
        }
    }

    ApplyConfiguredAssets();
}

void ABMEnemyBase::ApplyConfiguredAssets()
{
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        if (EnemyMeshAsset)
        {
            MeshComp->SetSkeletalMesh(EnemyMeshAsset);
        }
    }
}

FName ABMEnemyBase::ResolveEnemyDataId() const
{
    if (!EnemyDataId.IsNone())
    {
        return EnemyDataId;
    }

    if (EnemyType != EBMEnemyType::None)
    {
        if (const UEnum* EnemyEnum = StaticEnum<EBMEnemyType>())
        {
            const FString EnumName = EnemyEnum->GetNameStringByValue(static_cast<int64>(EnemyType));
            if (!EnumName.IsEmpty())
            {
                return FName(*EnumName);
            }
        }
    }

    return NAME_None;
}

void ABMEnemyBase::CachePlayerPawn()
{
    if (UWorld* World = GetWorld())
    {
        CachedPlayer = UGameplayStatics::GetPlayerPawn(World, 0);
    }
}
