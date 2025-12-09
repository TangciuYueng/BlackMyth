#include "Character/BMCharacterBase.h"

#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/Components/BMAnimEventComponent.h"
#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"

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
    // 默认：不吃自己打自己的伤害
    if (Info.InstigatorActor.Get() == this)
    {
        return false;
    }

    // 默认不做阵营过滤
    return true;
}

float ABMCharacterBase::GetDamageMultiplierForComponent(const UPrimitiveComponent* HitComponent) const
{
    if (!HitComponent)
    {
        return 1.f;
    }

    for (const TObjectPtr<UBMHurtBoxComponent>& HB : HurtBoxes)
    {
        if (HB && HB->IsBoundTo(HitComponent))
        {
            return HB->GetDamageMultiplier();
        }
    }

    return 1.f;
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

    // 统一：RawDamageValue 作为原始输入，DamageValue 作为可变通道
    const float Base = (InOutInfo.RawDamageValue > 0.f) ? InOutInfo.RawDamageValue : InOutInfo.DamageValue;
    InOutInfo.RawDamageValue = Base;
    InOutInfo.DamageValue = Base;

    // 1) 组件倍率（若没 HurtBox，就默认 1）
    if (UPrimitiveComponent* HitComp = InOutInfo.HitComponent.Get())
    {
        InOutInfo.DamageValue *= GetDamageMultiplierForComponent(HitComp);

        // 2) HurtBox 更细粒度修改（弱抗/额外逻辑）
        for (const TObjectPtr<UBMHurtBoxComponent>& HB : HurtBoxes)
        {
            if (HB && HB->IsBoundTo(HitComp))
            {
                HB->ModifyIncomingDamage(InOutInfo);
                break;
            }
        }
    }

    // 3) Stats 最终结算（防御/扣血/死亡）
    const float Applied = Stats->ApplyDamage(InOutInfo);

    if (Applied > 0.f)
    {
        LastAppliedDamageInfo = InOutInfo;

        HandleDamageTaken(InOutInfo);
        OnCharacterDamaged.Broadcast(this, InOutInfo);
    }

    return Applied;
}

void ABMCharacterBase::HandleDamageTaken(const FBMDamageInfo& FinalInfo)
{
    // 基类只做轻量日志，派生类可 override
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
    // 留给派生类：敌人掉落/玩家复活/切UI/禁用输入等
}
