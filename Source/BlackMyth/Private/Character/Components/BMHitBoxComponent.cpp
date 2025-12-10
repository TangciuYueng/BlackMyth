#include "Character/Components/BMHitBoxComponent.h"

#include "Character/BMCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"

DEFINE_LOG_CATEGORY(LogBMHitBox);

UBMHitBoxComponent::UBMHitBoxComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // 默认配置
    FBMHitBoxConfig Default;
    Default.BaseDamage = 10.f;
    Default.DamageType = EBMDamageType::Melee;
    Default.ElementType = EBMElementType::Physical;
    Default.DefaultReaction = EBMHitReaction::Light;
    Default.KnockbackStrength = 0.f;

    Configs.Add(EBMHitBoxType::Default, Default);
    Configs.Add(EBMHitBoxType::LightAttack, Default);
    Configs.Add(EBMHitBoxType::HeavyAttack, Default);
    Configs.Add(EBMHitBoxType::Skill, Default);
}

void UBMHitBoxComponent::BeginPlay()
{
    Super::BeginPlay();
    EnsureDefaultHitBoxCreated();
}

ABMCharacterBase* UBMHitBoxComponent::GetOwnerCharacter() const
{
    return Cast<ABMCharacterBase>(GetOwner());
}

void UBMHitBoxComponent::EnsureDefaultHitBoxCreated()
{
    if (HitBoxes.Contains(EBMHitBoxType::Default))
    {
        return;
    }

    ABMCharacterBase* OwnerChar = GetOwnerCharacter();
    if (!OwnerChar)
    {
        UE_LOG(LogBMHitBox, Error, TEXT("HitBoxComponent owner is not ABMCharacterBase."));
        return;
    }

    // 运行时创建默认 HitBox（纯C++）
    UBoxComponent* Box = NewObject<UBoxComponent>(OwnerChar, TEXT("HitBox_Default"));
    if (!Box) return;

    USceneComponent* AttachParent = OwnerChar->GetMesh() ? (USceneComponent*)OwnerChar->GetMesh() : OwnerChar->GetRootComponent();
    Box->SetupAttachment(AttachParent);

    Box->SetBoxExtent(FVector(10.f, 10.f, 10.f));
    Box->SetGenerateOverlapEvents(true);
    Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Box->SetCollisionObjectType(ECC_WorldDynamic);
    Box->SetCollisionResponseToAllChannels(ECR_Ignore);
    Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    Box->OnComponentBeginOverlap.AddDynamic(this, &UBMHitBoxComponent::OnHitBoxOverlap);

    Box->RegisterComponent();

    HitBoxes.Add(EBMHitBoxType::Default, Box);
}

void UBMHitBoxComponent::SetConfig(EBMHitBoxType Type, const FBMHitBoxConfig& InConfig)
{
    Configs.Add(Type, InConfig);
}

const FBMHitBoxConfig& UBMHitBoxComponent::GetConfigForType(EBMHitBoxType Type) const
{
    if (const FBMHitBoxConfig* Found = Configs.Find(Type))
    {
        return *Found;
    }
    return Configs.FindChecked(EBMHitBoxType::Default);
}

void UBMHitBoxComponent::ActivateHitBox(EBMHitBoxType Type)
{
    EnsureDefaultHitBoxCreated();

    ActiveType = Type;

    TObjectPtr<UBoxComponent>* Found = HitBoxes.Find(Type);
    if (!Found || !(*Found))
    {
        Found = HitBoxes.Find(EBMHitBoxType::Default);
    }

    ActiveBox = Found ? *Found : nullptr;

    if (ActiveBox)
    {
        HitActorsThisSwing.Reset();
        ActiveBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
}

void UBMHitBoxComponent::DeactivateHitBox()
{
    if (ActiveBox)
    {
        ActiveBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ActiveBox = nullptr;
    }
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

    if (!ActiveBox || !OtherActor || OtherActor == GetOwner())
    {
        return;
    }

    if (HitActorsThisSwing.Contains(OtherActor))
    {
        return;
    }

    ABMCharacterBase* InstigatorChar = GetOwnerCharacter();
    ABMCharacterBase* VictimChar = Cast<ABMCharacterBase>(OtherActor);
    if (!InstigatorChar || !VictimChar)
    {
        return;
    }

    HitActorsThisSwing.Add(OtherActor);

    const FBMHitBoxConfig& Cfg = GetConfigForType(ActiveType);

    FBMDamageInfo Info;
    Info.InstigatorActor = InstigatorChar;
    Info.TargetActor = VictimChar;

    // 统一：原始伤害写 RawDamageValue，DamageValue 作为“可变的计算通道”
    Info.RawDamageValue = Cfg.BaseDamage;
    Info.DamageValue = Cfg.BaseDamage;

    Info.DamageType = Cfg.DamageType;
    Info.ElementType = Cfg.ElementType;

    // 默认受击反馈（Victim 侧也可进一步改）
    Info.HitReaction = Cfg.DefaultReaction;

    // 命中组件/点/法线
    Info.HitComponent = OtherComp;

    if (bFromSweep)
    {
        Info.HitLocation = SweepResult.ImpactPoint;
        Info.HitNormal = SweepResult.ImpactNormal;
    }
    else
    {
        Info.HitLocation = OtherComp ? OtherComp->GetComponentLocation() : VictimChar->GetActorLocation();
        Info.HitNormal = FVector::UpVector;
    }

    // 击退
    if (Cfg.KnockbackStrength > 0.f)
    {
        const FVector Dir = (VictimChar->GetActorLocation() - InstigatorChar->GetActorLocation()).GetSafeNormal();
        Info.Knockback = Dir * Cfg.KnockbackStrength;
    }

    // 关键：Victim 侧会“回填” Info.DamageValue（最终扣血量等）
    const float Applied = VictimChar->TakeDamageFromHit(Info);

    if (Applied > 0.f)
    {
        OnHitLanded.Broadcast(InstigatorChar, VictimChar, Info);
    }
}
