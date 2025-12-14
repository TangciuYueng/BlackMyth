#include "Character/Components/BMHitBoxComponent.h"

#include "Character/BMCharacterBase.h"
#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY(LogBMHitBox);

UBMHitBoxComponent::UBMHitBoxComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UBMHitBoxComponent::BeginPlay()
{
    Super::BeginPlay();

    // 若用户没注册，创建一个默认定义
    if (Definitions.Num() == 0)
    {
        FBMHitBoxDefinition Def;
        Def.Name = TEXT("Default");
        Def.Type = EBMHitBoxType::Default;
        Def.AttachSocketOrBone = NAME_None;
        Def.BoxExtent = FVector(8.f);
        Def.DamageType = EBMDamageType::Melee;
        Def.ElementType = ElementType;
        Definitions.Add(Def);
    }

    for (const FBMHitBoxDefinition& Def : Definitions)
    {
        EnsureCreated(Def);
    }

    DeactivateHitBox();
}

void UBMHitBoxComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (auto& KVP : HitBoxes)
    {
        if (KVP.Value)
        {
            KVP.Value->DestroyComponent();
        }
    }
    HitBoxes.Empty();
    ActiveHitBox = nullptr;
    HitActorsThisSwing.Reset();

    Super::EndPlay(EndPlayReason);
}

void UBMHitBoxComponent::RegisterDefinition(const FBMHitBoxDefinition& Def)
{
    Definitions.Add(Def);
}

USkeletalMeshComponent* UBMHitBoxComponent::ResolveOwnerMesh() const
{
    if (ACharacter* C = Cast<ACharacter>(GetOwner()))
    {
        return C->GetMesh();
    }
    return nullptr;
}

ABMCharacterBase* UBMHitBoxComponent::ResolveOwnerCharacter() const
{
    return Cast<ABMCharacterBase>(GetOwner());
}

FName UBMHitBoxComponent::TypeToName(EBMHitBoxType Type)
{
    // 用 FName 做 key
    switch (Type)
    {
        case EBMHitBoxType::LightAttack: return TEXT("LightAttack");
        case EBMHitBoxType::HeavyAttack: return TEXT("HeavyAttack");
        case EBMHitBoxType::Skill:       return TEXT("Skill");
        default:                         return TEXT("Default");
    }
}

const FBMHitBoxDefinition* UBMHitBoxComponent::FindDefByType(EBMHitBoxType Type) const
{
    for (const FBMHitBoxDefinition& Def : Definitions)
    {
        if (Def.Type == Type)
        {
            return &Def;
        }
    }
    // 回退 Default
    for (const FBMHitBoxDefinition& Def : Definitions)
    {
        if (Def.Type == EBMHitBoxType::Default)
        {
            return &Def;
        }
    }
    return nullptr;
}

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

    const FName CompName = MakeUniqueObjectName(Owner, UBoxComponent::StaticClass(), *FString::Printf(TEXT("BM_HitBox_%s"), *Def.Name.ToString()));
    UBoxComponent* Box = NewObject<UBoxComponent>(Owner, CompName);
    Owner->AddInstanceComponent(Box);
    Box->RegisterComponent();

    Box->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform, Def.AttachSocketOrBone);
    Box->SetBoxExtent(Def.BoxExtent);
    Box->SetRelativeTransform(Def.RelativeTransform);

    Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Box->SetGenerateOverlapEvents(true);

    // 只 Overlap WorldDynamic（避免 Pawn/Capsule 重复触发）
    Box->SetCollisionObjectType(ECC_WorldDynamic);
    Box->SetCollisionResponseToAllChannels(ECR_Ignore);
    Box->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

    Box->ComponentTags.Add(TEXT("BM_HitBox"));
    Box->OnComponentBeginOverlap.AddDynamic(this, &UBMHitBoxComponent::OnHitBoxOverlap);

    HitBoxes.Add(Def.Name, Box);


}

void UBMHitBoxComponent::ActivateHitBox(EBMHitBoxType Type)
{
    const FString TypeStr = UEnum::GetValueAsString(Type);

    // 先打印请求启用
    UE_LOG(LogBMHitBox, Warning, TEXT("[HitBox] Activate request: Type=%s Owner=%s"),
        *TypeStr,
        *GetNameSafe(GetOwner()));

    const FBMHitBoxDefinition* Def = FindDefByType(Type);
    if (!Def)
    {
        UE_LOG(LogBMHitBox, Error, TEXT("[HitBox] Activate failed: no definition for Type=%s"), *TypeStr);
        return;
    }

    UE_LOG(LogBMHitBox, Warning, TEXT("[HitBox] Found Def: Name=%s Type=%s Socket/Bone=%s Extent=%s"),
        *Def->Name.ToString(),
        *UEnum::GetValueAsString(Def->Type),
        *Def->AttachSocketOrBone.ToString(),
        *Def->BoxExtent.ToString());

    if (Def->Name.IsNone())
    {
        UE_LOG(LogBMHitBox, Error, TEXT("[HitBox] Def->Name is None! (Type=%s) Please ensure Name is set or auto-filled."), *TypeStr);
    }

    EnsureCreated(*Def);

    ActiveHitBoxName = Def->Name;

    // 用 FindRef 更清晰
    ActiveHitBox = HitBoxes.FindRef(ActiveHitBoxName);

    // 打印当前 HitBoxes 里有哪些 key
    {
        FString Keys;
        for (const auto& KVP : HitBoxes)
        {
            Keys += KVP.Key.ToString() + TEXT(" ");
        }

        UE_LOG(LogBMHitBox, Warning, TEXT("[HitBox] Map Keys=%s | ActiveKey=%s"),
            *Keys,
            *ActiveHitBoxName.ToString());
    }

    HitActorsThisSwing.Reset();

    if (!ActiveHitBox)
    {
        UE_LOG(LogBMHitBox, Error, TEXT("[HitBox] Activate failed: ActiveHitBox is null. ActiveName=%s Type=%s"),
            *ActiveHitBoxName.ToString(), *TypeStr);
        return;
    }

    ActiveHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    UE_LOG(LogBMHitBox, Warning, TEXT("[HitBox] ON: Name=%s Type=%s Comp=%s Collision=%d"),
        *ActiveHitBoxName.ToString(),
        *TypeStr,
        *GetNameSafe(ActiveHitBox),
        (int32)ActiveHitBox->GetCollisionEnabled());
}

void UBMHitBoxComponent::DeactivateHitBox()
{
    if (!ActiveHitBox)
    {
        UE_LOG(LogBMHitBox, Display, TEXT("[HitBox] OFF: (no active) Owner=%s ActiveName=%s"),
            *GetNameSafe(GetOwner()),
            *ActiveHitBoxName.ToString());

        ActiveHitBoxName = NAME_None;
        HitActorsThisSwing.Reset();
        return;
    }

    const int32 Before = (int32)ActiveHitBox->GetCollisionEnabled();

    UE_LOG(LogBMHitBox, Warning, TEXT("[HitBox] OFF request: Name=%s Comp=%s CollisionBefore=%d"),
        *ActiveHitBoxName.ToString(),
        *GetNameSafe(ActiveHitBox),
        Before);

    ActiveHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    const int32 After = (int32)ActiveHitBox->GetCollisionEnabled();

    UE_LOG(LogBMHitBox, Warning, TEXT("[HitBox] OFF done: Name=%s Comp=%s CollisionAfter=%d"),
        *ActiveHitBoxName.ToString(),
        *GetNameSafe(ActiveHitBox),
        After);

    ActiveHitBox = nullptr;
    ActiveHitBoxName = NAME_None;
    HitActorsThisSwing.Reset();
}


void UBMHitBoxComponent::ResetHitList()
{
    HitActorsThisSwing.Reset();
}

void UBMHitBoxComponent::OnHitBoxOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    (void)OverlappedComponent;
    (void)OtherBodyIndex;

    if (!ActiveHitBox || !OtherActor || OtherActor == GetOwner())
    {
        return;
    }

    // 一次挥砍窗口同一目标只结算一次
    if (HitActorsThisSwing.Contains(OtherActor))
    {
        return;
    }

    ABMCharacterBase* Attacker = ResolveOwnerCharacter();
    ABMCharacterBase* Victim = Cast<ABMCharacterBase>(OtherActor);
    if (!Attacker || !Victim)
    {
        return;
    }

    // 必须命中对方的 HurtBox（避免打到 Capsule/其它组件）
    if (!OtherComp || !OtherComp->ComponentHasTag(TEXT("BM_HurtBox")))
    {
        return;
    }

    HitActorsThisSwing.Add(OtherActor);

    // 取当前 HitBox 定义
    const FBMHitBoxDefinition* Def = nullptr;
    for (const FBMHitBoxDefinition& D : Definitions)
    {
        if (D.Name == ActiveHitBoxName)
        {
            Def = &D;
            break;
        }
    }
    if (!Def)
    {
        return;
    }

    // 计算基础伤害：优先 HitBoxComponent.Damage 覆盖，否则用 Attacker Stats.Attack
    float BaseAttack = 0.f;
    if (Damage > 0.f)
    {
        BaseAttack = Damage;
    }
    else if (UBMStatsComponent* S = Attacker->GetStats())
    {
        BaseAttack = S->GetStatBlock().Attack;
    }

    FBMDamageInfo Info;
    Info.InstigatorActor = Attacker;
    Info.TargetActor = Victim;

    Info.RawDamageValue = BaseAttack;
    Info.DamageValue = BaseAttack * Def->DamageScale + Def->AdditiveDamage;

    Info.DamageType = Def->DamageType;
    Info.ElementType = Def->ElementType;
    Info.HitReaction = Def->DefaultReaction;

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

    if (Def->KnockbackStrength > 0.f)
    {
        const FVector Dir = (Victim->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();
        Info.Knockback = Dir * Def->KnockbackStrength;
    }

    // Victim 侧会执行 HurtBox->ModifyIncomingDamage + Stats->ApplyDamage（并回填 DamageValue 为最终扣血）
    const float Applied = Victim->TakeDamageFromHit(Info);

    // Applied==0 代表被过滤/被免疫/没扣血，不需要额外处理
    (void)Applied;
}

void UBMHitBoxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bDebugDraw) return;

    for (const auto& KVP : HitBoxes)
    {
        UBoxComponent* Box = KVP.Value;
        if (!Box) continue;

        const bool bIsCurrentActiveHitBox = (Box == ActiveHitBox);
        const FColor Color = bIsCurrentActiveHitBox ? DebugColorActive : DebugColorInactive;

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
