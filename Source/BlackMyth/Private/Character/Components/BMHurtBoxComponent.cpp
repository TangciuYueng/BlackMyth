#include "Character/Components/BMHurtBoxComponent.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY(LogBMHurtBox);

UBMHurtBoxComponent::UBMHurtBoxComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UBMHurtBoxComponent::BeginPlay()
{
    Super::BeginPlay();
    CreateOrUpdateCollision();
}

void UBMHurtBoxComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (CollisionBox)
    {
        CollisionBox->DestroyComponent();
        CollisionBox = nullptr;
        BoundComponent.Reset();
    }

    Super::EndPlay(EndPlayReason);
}

USkeletalMeshComponent* UBMHurtBoxComponent::ResolveOwnerMesh() const
{
    if (ACharacter* C = Cast<ACharacter>(GetOwner()))
    {
        return C->GetMesh();
    }
    return nullptr;
}

void UBMHurtBoxComponent::CreateOrUpdateCollision()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogBMHurtBox, Warning, TEXT("HurtBox has no owner."));
        return;
    }

    USkeletalMeshComponent* Mesh = ResolveOwnerMesh();
    if (!Mesh)
    {
        UE_LOG(LogBMHurtBox, Warning, TEXT("[%s] HurtBox cannot recall mesh."), *Owner->GetName());
        return;
    }

    if (!CollisionBox)
    {
        const FName CompName = MakeUniqueObjectName(Owner, UBoxComponent::StaticClass(), TEXT("BM_HurtBox"));
        CollisionBox = NewObject<UBoxComponent>(Owner, CompName);

        Owner->AddInstanceComponent(CollisionBox);
        CollisionBox->RegisterComponent();
    }

    CollisionBox->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform, AttachSocketOrBone);

    CollisionBox->SetBoxExtent(BoxExtent);
    CollisionBox->SetRelativeTransform(RelativeTransform);

    // 只参与 Overlap（不阻挡移动）
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionBox->SetGenerateOverlapEvents(true);

    // 与 HitBox 统一用 WorldDynamic
    CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

    CollisionBox->ComponentTags.Add(TEXT("BM_HurtBox"));

    BoundComponent = CollisionBox;
}

void UBMHurtBoxComponent::ModifyIncomingDamage(FBMDamageInfo& InOutInfo) const
{
    // 1) 部位倍率
    InOutInfo.DamageValue *= DamageMultiplier;

    // 2) 弱点/抗性
    if (WeaknessTypes.Contains(InOutInfo.ElementType))
    {
        InOutInfo.DamageValue *= 1.25f;
    }
    if (ResistanceTypes.Contains(InOutInfo.ElementType))
    {
        InOutInfo.DamageValue *= 0.75f;
    }

    // 3) 推荐受击反馈：倍率高时偏重
    if (InOutInfo.HitReaction == EBMHitReaction::None)
    {
        InOutInfo.HitReaction = (DamageMultiplier >= 1.5f) ? EBMHitReaction::Heavy : EBMHitReaction::Light;
    }
}

void UBMHurtBoxComponent::OnHit(float AppliedDamage)
{
    (void)AppliedDamage;
}

void UBMHurtBoxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{   
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bDebugDraw || !CollisionBox) return;

    const FVector Center = CollisionBox->GetComponentLocation();
    const FVector Extent = CollisionBox->GetScaledBoxExtent();
    const FQuat   Rot = CollisionBox->GetComponentQuat();

    DrawDebugBox(
        GetWorld(),
        Center,
        Extent,
        Rot,
        DebugColor,
        false,   // bPersistentLines
        0.0f,    // LifeTime = 0 表示仅一帧
        0,
        DebugLineThickness
    );
}