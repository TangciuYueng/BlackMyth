#include "Character/Components/BMHurtBoxComponent.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY(LogBMHurtBox);

/*
 * @brief Constructor of the UBMHurtBoxComponent class
 */
UBMHurtBoxComponent::UBMHurtBoxComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

/*
 * @brief Begin play, it creates or updates the collision
 */
void UBMHurtBoxComponent::BeginPlay()
{
    Super::BeginPlay();
    CreateOrUpdateCollision();
}

/*
 * @brief End play, it destroys the collision and resets the bound component
 * @param EndPlayReason The reason for the end play
 */
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

/*
 * @brief Resolve owner mesh, it resolves the owner mesh from the owner
 * @return The owner mesh
 */
USkeletalMeshComponent* UBMHurtBoxComponent::ResolveOwnerMesh() const
{
    if (ACharacter* C = Cast<ACharacter>(GetOwner()))
    {
        return C->GetMesh();
    }
    return nullptr;
}

/*
 * @brief Create or update collision, it creates or updates the collision
 */
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

    // ֻ���� Overlap
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionBox->SetGenerateOverlapEvents(true);

    // �� HitBox ͳһ�� WorldDynamic
    CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

    CollisionBox->ComponentTags.Add(TEXT("BM_HurtBox"));

    BoundComponent = CollisionBox;
}

/*
 * @brief Modify incoming damage, it modifies the incoming damage based on the damage multiplier and weakness/resistance types
 * @param InOutInfo The incoming damage info
 */
void UBMHurtBoxComponent::ModifyIncomingDamage(FBMDamageInfo& InOutInfo) const
{
    InOutInfo.DamageValue *= DamageMultiplier;

    if (WeaknessTypes.Contains(InOutInfo.ElementType))
    {
        InOutInfo.DamageValue *= 1.25f;
    }
    if (ResistanceTypes.Contains(InOutInfo.ElementType))
    {
        InOutInfo.DamageValue *= 0.75f;
    }
}

/*
 * @brief On hit, it handles the hit
 * @param AppliedDamage The applied damage
 */
void UBMHurtBoxComponent::OnHit(float AppliedDamage)
{
    (void)AppliedDamage;
}

/*
 * @brief Tick component, it ticks the component and draws the debug box if the bDebugDraw property is true
 * @param DeltaTime The delta time
 * @param TickType The tick type
 * @param ThisTickFunction The tick function
 */
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
        false,   
        0.0f,    
        0,
        DebugLineThickness
    );
}

/*
 * @brief Is hurt box enabled, it checks if the hurt box is enabled
 * @return True if the hurt box is enabled, false otherwise
 */
bool UBMHurtBoxComponent::IsHurtBoxEnabled() const
{
    return CollisionBox && CollisionBox->GetCollisionEnabled() != ECollisionEnabled::NoCollision;
}

/*
 * @brief Set hurt box enabled, it sets the hurt box enabled
 * @param bEnabled The enabled
 */
void UBMHurtBoxComponent::SetHurtBoxEnabled(bool bEnabled)
{
    if (!CollisionBox) return;

    CollisionBox->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    CollisionBox->SetGenerateOverlapEvents(bEnabled);
}