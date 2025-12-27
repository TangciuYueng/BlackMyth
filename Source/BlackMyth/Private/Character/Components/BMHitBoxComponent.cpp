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

    // 只 Overlap WorldDynamic
    Box->SetCollisionObjectType(ECC_WorldDynamic);
    Box->SetCollisionResponseToAllChannels(ECR_Ignore);
    Box->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

    Box->ComponentTags.Add(TEXT("BM_HitBox"));
    Box->OnComponentBeginOverlap.AddDynamic(this, &UBMHitBoxComponent::OnHitBoxOverlap);

    HitBoxes.Add(Def.Name, Box);
    ComponentToHitBoxName.Add(Box, Def.Name);
}

void UBMHitBoxComponent::ResetHitList()
{
    HitRecordsThisWindow.Reset();
}

void UBMHitBoxComponent::OnHitBoxOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    (void)OtherBodyIndex;

    // 基础合法性检查
    if (!OverlappedComponent || !OtherActor || OtherActor == GetOwner())
    {
        return;
    }

    // 必须命中对方的 HurtBox
    if (!OtherComp || !OtherComp->ComponentHasTag(TEXT("BM_HurtBox")))
    {
        return;
    }

    // 通过 OverlappedComponent 找到HitBox
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

    // 当前窗口激活的HitBoxNames
    if (!ActiveHitBoxNames.Contains(HitBoxName))
    {
        return;
    }

    // 解析攻击者/受害者
    ABMCharacterBase* Attacker = ResolveOwnerCharacter();
    ABMCharacterBase* Victim = Cast<ABMCharacterBase>(OtherActor);
    if (!Attacker || !Victim)
    {
        return;
    }

    // 去重
    TWeakObjectPtr<AActor> TargetKey(OtherActor);
    FBMHitRecord& Record = HitRecordsThisWindow.FindOrAdd(TargetKey);

    // 读取去重策略
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

    // 通过去重检查后，先记一次命中
    Record.TotalHits++;
    Record.HitBoxHits.FindOrAdd(HitBoxName)++;

    // 定位HitBoxDefinition
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

    // 计算基础伤害
    float BaseAttack = 0.f;
    float AttackMultiplier = 1.0f;

    if (Damage > 0.f)
    {
        BaseAttack = Damage;
    }
    else if (UBMStatsComponent* S = Attacker->GetStats())
    {
        BaseAttack = S->GetStatBlock().Attack;
        // 获取攻击力加成倍率
        AttackMultiplier = S->GetAttackMultiplier();
    }

    // 应用攻击力加成
    BaseAttack *= AttackMultiplier;

    // 构造FBMDamageInfo
    FBMDamageInfo Info;
    Info.InstigatorActor = Attacker;
    Info.TargetActor = Victim;

    Info.RawDamageValue = BaseAttack;

    // Def：BaseAttack * DamageScale + AdditiveDamage
    // 再乘以这次攻击窗口倍率
    const float DefDamage = BaseAttack * Def->DamageScale + Def->AdditiveDamage;
    Info.DamageValue = DefDamage * FMath::Max(0.f, ActiveWindowParams.DamageMultiplier);


    Info.DamageType = Def->DamageType;

    // 受击反馈
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

    // 结算
    Victim->TakeDamageFromHit(Info);
}


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

void UBMHitBoxComponent::SetHitBoxCollisionEnabled(FName HitBoxName, bool bEnabled)
{
    if (HitBoxName.IsNone()) return;

    if (UBoxComponent* Box = HitBoxes.FindRef(HitBoxName))
    {
        Box->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}

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

        // 若没创建
        const int32* Index = NameToDefIndex.Find(Name);
        if (Index && Definitions.IsValidIndex(*Index))
        {
            EnsureCreated(Definitions[*Index]);
        }

        // 激活
        ActiveHitBoxNames.Add(Name);
        SetHitBoxCollisionEnabled(Name, true);
    }
}

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

