#include "Character/Components/BMHitBoxComponent.h"

#include "Character/BMCharacterBase.h"
#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY(LogBMHitBox);

/*
 * @brief Constructor of the UBMHitBoxComponent class
 */
UBMHitBoxComponent::UBMHitBoxComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

/*
 * @brief Begin play, it initializes the name to def index and component to hit box name
 */
void UBMHitBoxComponent::BeginPlay()
{
    Super::BeginPlay();

    NameToDefIndex.Empty();
    ComponentToHitBoxName.Empty();

    if (Definitions.Num() == 0)
    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("Default");
        Def.Type = EBMHitBoxType::Default;
        Def.AttachSocketOrBone = NAME_None;
        Def.BoxExtent = FVector(8.f);
        Def.DamageType = EBMDamageType::Melee;
        Definitions.Add(Def);
    }

    for (int32 i = 0; i < Definitions.Num(); ++i)
    {
        const FBMHitBoxDefinition& Def = Definitions[i];
        EnsureCreated(Def);

        if (!Def.Name.IsNone())
        {
            NameToDefIndex.Add(Def.Name, i);
        }
    }

    DeactivateAllHitBoxes();
}

/*
 * @brief End play, it deactivates all hit boxes and destroys the hit boxes
 * @param EndPlayReason The reason for the end play
 */
void UBMHitBoxComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DeactivateAllHitBoxes();

    for (auto& KVP : HitBoxes)
    {
        if (KVP.Value)
        {
            KVP.Value->OnComponentBeginOverlap.RemoveAll(this);
            KVP.Value->DestroyComponent();
        }
    }

    HitBoxes.Empty();
    ComponentToHitBoxName.Empty();
    NameToDefIndex.Empty();

    ActiveHitBoxNames.Reset();
    HitRecordsThisWindow.Reset();
    ActiveWindowParams = FBMHitBoxActivationParams();

    Super::EndPlay(EndPlayReason);
}

/*
 * @brief Register definition, it adds the definition to the definitions array
 * @param Def The definition to register
 */
void UBMHitBoxComponent::RegisterDefinition(const FBMHitBoxDefinition& Def)
{
    Definitions.Add(Def);
}

/*
 * @brief Resolve owner mesh, it resolves the owner mesh from the owner
 * @return The owner mesh
 */
USkeletalMeshComponent* UBMHitBoxComponent::ResolveOwnerMesh() const
{
    if (ACharacter* C = Cast<ACharacter>(GetOwner()))
    {
        return C->GetMesh();
    }
    return nullptr;
}

/*
 * @brief Resolve owner character, it resolves the owner character from the owner
 * @return The owner character
 */
ABMCharacterBase* UBMHitBoxComponent::ResolveOwnerCharacter() const
{
    return Cast<ABMCharacterBase>(GetOwner());
}

/*
 * @brief Type to name, it converts the type to a name
 * @param Type The type to convert
 * @return The name
 */
FName UBMHitBoxComponent::TypeToName(EBMHitBoxType Type)
{
    // �� FName �� key
    switch (Type)
    {
        case EBMHitBoxType::LightAttack: return TEXT("LightAttack");
        case EBMHitBoxType::HeavyAttack: return TEXT("HeavyAttack");
        case EBMHitBoxType::Skill:       return TEXT("Skill");
        default:                         return TEXT("Default");
    }
}

/*
 * @brief Find def by type, it finds the definition by type
 * @param Type The type to find
 * @return The definition
 */
const FBMHitBoxDefinition* UBMHitBoxComponent::FindDefByType(EBMHitBoxType Type) const
{
    for (const FBMHitBoxDefinition& Def : Definitions)
    {
        if (Def.Type == Type)
        {
            return &Def;
        }
    }
    // ���� Default
    for (const FBMHitBoxDefinition& Def : Definitions)
    {
        if (Def.Type == EBMHitBoxType::Default)
        {
            return &Def;
        }
    }
    return nullptr;
}

/*
 * @brief Ensure created, it ensures that the hit box is created
 * @param Def The definition to ensure
 */
void UBMHitBoxComponent::EnsureCreated(const FBMHitBoxDefinition& Def)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (Def.Name.IsNone())
    {
        UE_LOG(LogBMHitBox, Warning, TEXT("[%s] HitBoxDefinition has None name."), *Owner->GetName());
        return;
    }

    if (HitBoxes.Contains(Def.Name))
    {
        return;
    }

    USkeletalMeshComponent* Mesh = ResolveOwnerMesh();
    if (!Mesh)
    {
        UE_LOG(LogBMHitBox, Warning, TEXT("[%s] HitBox cannot resolve mesh."), *Owner->GetName());
        return;
    }

    // Debug
    if (Def.AttachSocketOrBone != NAME_None && !Mesh->DoesSocketExist(Def.AttachSocketOrBone))
    {
        UE_LOG(
            LogBMHitBox,
            Warning,
            TEXT("[%s] HitBox '%s' AttachSocketOrBone '%s' does NOT exist on mesh '%s'."),
            *Owner->GetName(),
            *Def.Name.ToString(),
            *Def.AttachSocketOrBone.ToString(),
            *Mesh->GetName()
        );
    }

    const FName CompName = MakeUniqueObjectName(Owner, UBoxComponent::StaticClass(), *FString::Printf(TEXT("BM_HitBox_%s"), *Def.Name.ToString()));
    UBoxComponent* Box = NewObject<UBoxComponent>(Owner, CompName);
    Owner->AddInstanceComponent(Box);
    Box->RegisterComponent();

    Box->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform, Def.AttachSocketOrBone);
    Box->SetBoxExtent(Def.BoxExtent);
    Box->SetRelativeTransform(Def.RelativeTransform);

    Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Box->SetGenerateOverlapEvents(true);

    // ֻ Overlap WorldDynamic
    Box->SetCollisionObjectType(ECC_WorldDynamic);
    Box->SetCollisionResponseToAllChannels(ECR_Ignore);
    Box->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

    Box->ComponentTags.Add(TEXT("BM_HitBox"));
    Box->OnComponentBeginOverlap.AddDynamic(this, &UBMHitBoxComponent::OnHitBoxOverlap);

    HitBoxes.Add(Def.Name, Box);
    ComponentToHitBoxName.Add(Box, Def.Name);
}

/*
 * @brief Reset hit list, it resets the hit list
 */
void UBMHitBoxComponent::ResetHitList()
{
    HitRecordsThisWindow.Reset();
}

/*
 * @brief On hit box overlap, it handles the hit box overlap
 * @param OverlappedComponent The overlapped component
 * @param OtherActor The other actor
 * @param OtherComp The other component
 * @param OtherBodyIndex The other body index
 * @param bFromSweep The from sweep
 * @param SweepResult The sweep result
 */
void UBMHitBoxComponent::OnHitBoxOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    (void)OtherBodyIndex;

    // �����Ϸ��Լ��
    if (!OverlappedComponent || !OtherActor || OtherActor == GetOwner())
    {
        return;
    }

    // �������жԷ��� HurtBox
    if (!OtherComp || !OtherComp->ComponentHasTag(TEXT("BM_HurtBox")))
    {
        return;
    }

    // ͨ�� OverlappedComponent �ҵ�HitBox
    const FName* HitBoxNamePtr = ComponentToHitBoxName.Find(OverlappedComponent);
    if (!HitBoxNamePtr)
    {
        return;
    }

    const FName HitBoxName = *HitBoxNamePtr;
    if (HitBoxName.IsNone())
    {
        return;
    }

    // ��ǰ���ڼ����HitBoxNames
    if (!ActiveHitBoxNames.Contains(HitBoxName))
    {
        return;
    }

    // ����������/�ܺ���
    ABMCharacterBase* Attacker = ResolveOwnerCharacter();
    ABMCharacterBase* Victim = Cast<ABMCharacterBase>(OtherActor);
    if (!Attacker || !Victim)
    {
        return;
    }

    // ȥ��
    TWeakObjectPtr<AActor> TargetKey(OtherActor);
    FBMHitRecord& Record = HitRecordsThisWindow.FindOrAdd(TargetKey);

    // ��ȡȥ�ز���
    switch (ActiveWindowParams.DedupPolicy)
    {
        case EBMHitDedupPolicy::PerWindow:
        {
            if (Record.TotalHits >= ActiveWindowParams.MaxHitsPerTarget)
            {
                return;
            }
            break;
        }
        case EBMHitDedupPolicy::PerHitBox:
        {
            const int32* Existing = Record.HitBoxHits.Find(HitBoxName);
            const int32 Cur = Existing ? *Existing : 0;
            if (Cur >= ActiveWindowParams.MaxHitsPerTarget)
            {
                return;
            }
            break;
        }
        case EBMHitDedupPolicy::Unlimited:
        default:
            break;
    }

    // ͨ��ȥ�ؼ����ȼ�һ������
    Record.TotalHits++;
    Record.HitBoxHits.FindOrAdd(HitBoxName)++;

    // ��λHitBoxDefinition
    const FBMHitBoxDefinition* Def = nullptr;

    if (const int32* DefIndex = NameToDefIndex.Find(HitBoxName))
    {
        if (Definitions.IsValidIndex(*DefIndex))
        {
            Def = &Definitions[*DefIndex];
        }
    }

    if (!Def)
    {
        for (const FBMHitBoxDefinition& D : Definitions)
        {
            if (D.Name == HitBoxName)
            {
                Def = &D;
                break;
            }
        }
    }

    if (!Def)
    {
        return;
    }

    // ��������˺�
    float BaseAttack = 0.f;
    float AttackMultiplier = 1.0f;

    if (Damage > 0.f)
    {
        BaseAttack = Damage;
    }
    else if (UBMStatsComponent* S = Attacker->GetStats())
    {
        BaseAttack = S->GetStatBlock().Attack;
        // ��ȡ�������ӳɱ���
        AttackMultiplier = S->GetAttackMultiplier();
    }

    // Ӧ�ù������ӳ�
    BaseAttack *= AttackMultiplier;

    // ����FBMDamageInfo
    FBMDamageInfo Info;
    Info.InstigatorActor = Attacker;
    Info.TargetActor = Victim;

    Info.RawDamageValue = BaseAttack;

    // Def��BaseAttack * DamageScale + AdditiveDamage
    // �ٳ�����ι������ڱ���
    const float DefDamage = BaseAttack * Def->DamageScale + Def->AdditiveDamage;
    Info.DamageValue = DefDamage * FMath::Max(0.f, ActiveWindowParams.DamageMultiplier);


    Info.DamageType = Def->DamageType;

    // �ܻ�����
    Info.HitReaction = Def->DefaultReaction;
    if (ActiveWindowParams.bOverrideReaction && ActiveWindowParams.OverrideReaction != EBMHitReaction::None)
    {
        Info.HitReaction = ActiveWindowParams.OverrideReaction;
    }

    Info.HitComponent = OtherComp;

    if (bFromSweep)
    {
        Info.HitLocation = SweepResult.ImpactPoint;
        Info.HitNormal = SweepResult.ImpactNormal;
    }
    else
    {
        Info.HitLocation = OtherComp->GetComponentLocation();
        Info.HitNormal = FVector::UpVector;
    }

    // ����
    Victim->TakeDamageFromHit(Info);
}


/*
 * @brief Tick component, it ticks the component and draws the debug boxes if the bDebugDraw property is true
 * @param DeltaTime The delta time
 * @param TickType The tick type
 * @param ThisTickFunction The tick function
 */
void UBMHitBoxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bDebugDraw) return;

    for (const auto& KVP : HitBoxes)
    {
        const FName Name = KVP.Key;
        UBoxComponent* Box = KVP.Value;
        if (!Box) continue;

        const bool bActive = ActiveHitBoxNames.Contains(Name);
        const FColor Color = bActive ? DebugColorActive : DebugColorInactive;

        DrawDebugBox(
            GetWorld(),
            Box->GetComponentLocation(),
            Box->GetScaledBoxExtent(),
            Box->GetComponentQuat(),
            Color,
            false,
            0.0f,
            0,
            DebugLineThickness
        );
    }
}

/*
 * @brief Set hit box collision enabled, it sets the hit box collision enabled
 * @param HitBoxName The hit box name
 * @param bEnabled The enabled
 */
void UBMHitBoxComponent::SetHitBoxCollisionEnabled(FName HitBoxName, bool bEnabled)
{
    if (HitBoxName.IsNone()) return;

    if (UBoxComponent* Box = HitBoxes.FindRef(HitBoxName))
    {
        Box->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}

/*
 * @brief Find names by type, it finds the names by type
 * @param Type The type to find
 * @return The names
 */
TArray<FName> UBMHitBoxComponent::FindNamesByType(EBMHitBoxType Type) const
{
    TArray<FName> Out;
    for (const FBMHitBoxDefinition& Def : Definitions)
    {
        if (Def.Type == Type && !Def.Name.IsNone())
        {
            Out.Add(Def.Name);
        }
    }
    return Out;
}

/*
 * @brief Activate hit boxes by names, it activates the hit boxes by names
 * @param HitBoxNames The hit box names
 * @param Params The activation parameters
 */
void UBMHitBoxComponent::ActivateHitBoxesByNames(const TArray<FName>& HitBoxNames, const FBMHitBoxActivationParams& Params)
{
    if (HitBoxNames.Num() == 0)
    {
        UE_LOG(LogBMHitBox, Warning, TEXT("[HitBox] ActivateHitBoxesByNames called with empty list."));
        return;
    }

    ActiveWindowParams = Params;

    if (ActiveWindowParams.bResetHitRecords)
    {
        HitRecordsThisWindow.Reset();
    }

    for (const FName& Name : HitBoxNames)
    {
        if (Name.IsNone()) continue;

        // ��û����
        const int32* Index = NameToDefIndex.Find(Name);
        if (Index && Definitions.IsValidIndex(*Index))
        {
            EnsureCreated(Definitions[*Index]);
        }

        // ����
        ActiveHitBoxNames.Add(Name);
        SetHitBoxCollisionEnabled(Name, true);
    }
}

/*
 * @brief Deactivate hit boxes by names, it deactivates the hit boxes by names
 * @param HitBoxNames The hit box names
 */
void UBMHitBoxComponent::DeactivateHitBoxesByNames(const TArray<FName>& HitBoxNames)
{
    for (const FName& Name : HitBoxNames)
    {
        if (Name.IsNone()) continue;

        SetHitBoxCollisionEnabled(Name, false);
        ActiveHitBoxNames.Remove(Name);
    }

    if (ActiveHitBoxNames.Num() == 0)
    {
        // ActiveWindowParams = FBMHitBoxActivationParams();
        // HitRecordsThisWindow.Reset(); 
    }
}

/*
 * @brief Deactivate all hit boxes, it deactivates all hit boxes
 */
void UBMHitBoxComponent::DeactivateAllHitBoxes()
{
    for (auto& KVP : HitBoxes)
    {
        if (KVP.Value)
        {
            KVP.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    ActiveHitBoxNames.Reset();
    ActiveWindowParams = FBMHitBoxActivationParams();
    HitRecordsThisWindow.Reset();
}

