#include "Character/BMCharacterBase.h"

#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMAnimEventComponent.h"
#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"
#include "Camera/BMCameraShakeSubsystem.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Animation/AnimInstance.h"
#include "Components/PrimitiveComponent.h"

DEFINE_LOG_CATEGORY(LogBMCharacter);

ABMCharacterBase::ABMCharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;

    Stats = CreateDefaultSubobject<UBMStatsComponent>(TEXT("Stats"));
    Combat = CreateDefaultSubobject<UBMCombatComponent>(TEXT("Combat"));
    FSM = CreateDefaultSubobject<UBMStateMachineComponent>(TEXT("FSM"));
    AnimEvent = CreateDefaultSubobject<UBMAnimEventComponent>(TEXT("AnimEvent"));
    HitBox = CreateDefaultSubobject<UBMHitBoxComponent>(TEXT("HitBox"));
}

void ABMCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    CacheHurtBoxes();

    if (ensure(Stats))
    {
        Stats->OnDeathNative.AddUObject(this, &ABMCharacterBase::HandleStatsDeath);
    }

    static const ECollisionChannel CameraBoomChannel = ECC_GameTraceChannel1; 

    TArray<UActorComponent*> Comps;
    GetComponents(UPrimitiveComponent::StaticClass(), Comps);

    for (UActorComponent* C : Comps)
    {
        UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(C);
        if (!Prim) continue;

        // 只改 trace 响应
        Prim->SetCollisionResponseToChannel(CameraBoomChannel, ECR_Ignore);
    }
}

void ABMCharacterBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (FSM)
    {
        FSM->TickState(DeltaSeconds);
    }
}

void ABMCharacterBase::CacheHurtBoxes()
{
    HurtBoxes.Reset();

    TArray<UBMHurtBoxComponent*> Found;
    GetComponents<UBMHurtBoxComponent>(Found);

    for (UBMHurtBoxComponent* HB : Found)
    {
        if (HB)
        {
            HurtBoxes.Add(HB);
        }
    }
}

FVector ABMCharacterBase::GetForwardVector() const
{
    return GetActorForwardVector();
}

bool ABMCharacterBase::CanBeDamagedBy(const FBMDamageInfo& Info) const
{
    // 不吃自己打自己的伤害
    if (Info.InstigatorActor.Get() == this)
    {
        return false;
    }

    // 同阵营不互相伤害
    const ABMCharacterBase* TempInstigator = Cast<ABMCharacterBase>(Info.InstigatorActor.Get());
    if (TempInstigator)
    {
        // 如果攻击者和受害者是同一阵营,则不造成伤害
        if (TempInstigator->Team == this->Team && this->Team != EBMTeam::Neutral)
        {
            return false;
        }
    }

    return true;
}

float ABMCharacterBase::TakeDamageFromHit(FBMDamageInfo& InOutInfo)
{
    if (!Stats)
    {
        UE_LOG(LogBMCharacter, Error, TEXT("[%s] Stats missing, cannot apply damage."), *GetName());
        InOutInfo.DamageValue = 0.f;
        return 0.f;
    }

    if (Stats->IsDead())
    {
        InOutInfo.DamageValue = 0.f;
        return 0.f;
    }

    // 确保 TargetActor 合理
    if (!InOutInfo.TargetActor)
    {
        InOutInfo.TargetActor = this;
    }
    // 基础过滤
    if (!CanBeDamagedBy(InOutInfo))
    {
        InOutInfo.DamageValue = 0.f;
        return 0.f;
    }
    if (TryEvadeIncomingHit(InOutInfo))
    {
        InOutInfo.DamageValue = 0.f;
        return 0.f;
    }
    UBMHurtBoxComponent* MatchedHB = nullptr;
    if (UPrimitiveComponent* HitComp = InOutInfo.HitComponent.Get())
    {
        for (const TObjectPtr<UBMHurtBoxComponent>& HB : HurtBoxes)
        {
            if (HB && HB->IsBoundTo(HitComp))
            {
                MatchedHB = HB;
                HB->ModifyIncomingDamage(InOutInfo);
                break;
            }
        }
    }

    // 3) Stats 最终结算（防御/扣血/死亡）
    const float Applied = Stats->ApplyDamage(InOutInfo);

    if (Applied > 0.f)
    {
        if (MatchedHB)
        {
            MatchedHB->OnHit(Applied);
        }

        LastAppliedDamageInfo = InOutInfo;
        HandleDamageTaken(InOutInfo);
        OnCharacterDamaged.Broadcast(this, InOutInfo);

        // 触发相机震动
        if (UWorld* World = GetWorld())
        {
            if (UBMCameraShakeSubsystem* ShakeSubsystem = World->GetSubsystem<UBMCameraShakeSubsystem>())
            {
                // 根据角色类型选择不同强度的震动
                switch (CharacterType)
                {
                case EBMCharacterType::Player:
                    ShakeSubsystem->PlayPlayerHitShake(InOutInfo);
                    break;
                case EBMCharacterType::Boss:
                    ShakeSubsystem->PlayBossHitShake(InOutInfo);
                    break;
                case EBMCharacterType::Enemy:
                default:
                    ShakeSubsystem->PlayEnemyHitShake(InOutInfo);
                    break;
                }
            }
        }
    }

    return Applied;
}

void ABMCharacterBase::HandleDamageTaken(const FBMDamageInfo& FinalInfo)
{
    UE_LOG(LogBMCharacter, Verbose, TEXT("[%s] Took %.2f damage from %s"),
        *GetName(),
        FinalInfo.DamageValue,
        FinalInfo.InstigatorActor ? *FinalInfo.InstigatorActor->GetName() : TEXT("None"));
}

void ABMCharacterBase::HandleStatsDeath(AActor* Killer)
{
    // 死亡时广播最后一次有效伤害信息
    if (!LastAppliedDamageInfo.TargetActor)
    {
        LastAppliedDamageInfo.TargetActor = this;
    }
    if (!LastAppliedDamageInfo.InstigatorActor)
    {
        LastAppliedDamageInfo.InstigatorActor = Killer;
    }

    HandleDeath(LastAppliedDamageInfo);
    OnCharacterDied.Broadcast(this, LastAppliedDamageInfo);
}

void ABMCharacterBase::HandleDeath(const FBMDamageInfo& LastHitInfo)
{
    (void)LastHitInfo;
}

bool ABMCharacterBase::GetActiveHitWindow(TArray<FName>& OutHitBoxNames, FBMHitBoxActivationParams& OutParams) const
{
    if (!bHasActiveHitWindow || ActiveHitWindowHitBoxes.Num() == 0)
    {
        return false;
    }
    OutHitBoxNames = ActiveHitWindowHitBoxes;
    OutParams = ActiveHitWindowParams;
    return true;
}

void ABMCharacterBase::SetActiveHitWindow(const TArray<FName>& InHitBoxNames, const FBMHitBoxActivationParams& InParams)
{
    ActiveHitWindowHitBoxes = InHitBoxNames;
    ActiveHitWindowParams = InParams;
    bHasActiveHitWindow = (ActiveHitWindowHitBoxes.Num() > 0);
}

void ABMCharacterBase::ClearActiveHitWindow()
{
    ActiveHitWindowHitBoxes.Reset();
    ActiveHitWindowParams = FBMHitBoxActivationParams();
    bHasActiveHitWindow = false;
}

bool ABMCharacterBase::ResolveHitBoxWindow(
    FName WindowId,
    TArray<FName>& OutHitBoxNames,
    FBMHitBoxActivationParams& OutParams
) const
{
    (void)WindowId;
    OutHitBoxNames.Reset();
    OutParams = FBMHitBoxActivationParams();
    return false;
}

void ABMCharacterBase::SetAllHurtBoxesEnabled(bool bEnabled)
{
	if (!HurtBoxes.Num()) CacheHurtBoxes();

    for (UBMHurtBoxComponent* HB : HurtBoxes)
    {
        if (!HB) continue;
        HB->SetHurtBoxEnabled(bEnabled);
    }
}

bool ABMCharacterBase::TryEvadeIncomingHit(const FBMDamageInfo& InInfo)
{
    (void)InInfo;
    return false; // 默认不闪避
}

