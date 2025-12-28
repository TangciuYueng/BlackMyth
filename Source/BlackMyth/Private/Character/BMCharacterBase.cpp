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

/*
 * @brief Constructor of the ABMCharacterBase class
 */
ABMCharacterBase::ABMCharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;

    Stats = CreateDefaultSubobject<UBMStatsComponent>(TEXT("Stats"));
    Combat = CreateDefaultSubobject<UBMCombatComponent>(TEXT("Combat"));
    FSM = CreateDefaultSubobject<UBMStateMachineComponent>(TEXT("FSM"));
    AnimEvent = CreateDefaultSubobject<UBMAnimEventComponent>(TEXT("AnimEvent"));
    HitBox = CreateDefaultSubobject<UBMHitBoxComponent>(TEXT("HitBox"));
}

/*
 * @brief Begin play, it begins the play
 */
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

        // ֻ�� trace ��Ӧ
        Prim->SetCollisionResponseToChannel(CameraBoomChannel, ECR_Ignore);
    }
}

/*
 * @brief Tick, it ticks the character base
 * @param DeltaSeconds The delta seconds
 */
void ABMCharacterBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (FSM)
    {
        FSM->TickState(DeltaSeconds);
    }
}

/*
 * @brief Cache hurt boxes, it caches the hurt boxes
 */
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

/*
 * @brief Get forward vector, it gets the forward vector
 * @return The forward vector
 */
FVector ABMCharacterBase::GetForwardVector() const
{
    return GetActorForwardVector();
}

/*
 * @brief Can be damaged by, it checks if the character can be damaged by the info
 * @param Info The info
 * @return True if the character can be damaged by the info, false otherwise
 */
bool ABMCharacterBase::CanBeDamagedBy(const FBMDamageInfo& Info) const
{
    // �����Լ����Լ����˺�
    if (Info.InstigatorActor.Get() == this)
    {
        return false;
    }

    // ͬ��Ӫ�������˺�
    const ABMCharacterBase* TempInstigator = Cast<ABMCharacterBase>(Info.InstigatorActor.Get());
    if (TempInstigator)
    {
        // ��������ߺ��ܺ�����ͬһ��Ӫ,������˺�
        if (TempInstigator->Team == this->Team && this->Team != EBMTeam::Neutral)
        {
            return false;
        }
    }

    return true;
}

/*
 * @brief Take damage from hit, it takes the damage from the hit
 * @param InOutInfo The in out info
 * @return The applied damage
 */
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

    // ȷ�� TargetActor ����
    if (!InOutInfo.TargetActor)
    {
        InOutInfo.TargetActor = this;
    }
    // ��������
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

    // Stats ���ս���
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

        // ���������
        if (UWorld* World = GetWorld())
        {
            if (UBMCameraShakeSubsystem* ShakeSubsystem = World->GetSubsystem<UBMCameraShakeSubsystem>())
            {
                // ���ݽ�ɫ����ѡ��ͬǿ�ȵ���
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

/*
 * @brief Handle damage taken, it handles the damage taken
 * @param FinalInfo The final info
 */
void ABMCharacterBase::HandleDamageTaken(const FBMDamageInfo& FinalInfo)
{
    UE_LOG(LogBMCharacter, Verbose, TEXT("[%s] Took %.2f damage from %s"),
        *GetName(),
        FinalInfo.DamageValue,
        FinalInfo.InstigatorActor ? *FinalInfo.InstigatorActor->GetName() : TEXT("None"));
}

/*
 * @brief Handle stats death, it handles the stats death
 * @param Killer The killer
 */
void ABMCharacterBase::HandleStatsDeath(AActor* Killer)
{
    // ����ʱ�㲥���һ����Ч�˺���Ϣ
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

/*
 * @brief Handle death, it handles the death
 * @param LastHitInfo The last hit info
 */
void ABMCharacterBase::HandleDeath(const FBMDamageInfo& LastHitInfo)
{
    (void)LastHitInfo;
}

/*
 * @brief Get active hit window, it gets the active hit window
 * @param OutHitBoxNames The out hit box names
 * @param OutParams The out params
 * @return True if the active hit window is resolved, false otherwise
 */
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

/*
 * @brief Set active hit window, it sets the active hit window
 * @param InHitBoxNames The in hit box names
 * @param InParams The in params
 */
void ABMCharacterBase::SetActiveHitWindow(const TArray<FName>& InHitBoxNames, const FBMHitBoxActivationParams& InParams)
{
    ActiveHitWindowHitBoxes = InHitBoxNames;
    ActiveHitWindowParams = InParams;
    bHasActiveHitWindow = (ActiveHitWindowHitBoxes.Num() > 0);
}

/*
 * @brief Clear active hit window, it clears the active hit window
 */
void ABMCharacterBase::ClearActiveHitWindow()
{
    ActiveHitWindowHitBoxes.Reset();
    ActiveHitWindowParams = FBMHitBoxActivationParams();
    bHasActiveHitWindow = false;
}

/*
 * @brief Resolve hit box window, it resolves the hit box window
 * @param WindowId The window id
 * @param OutHitBoxNames The out hit box names
 * @param OutParams The out params
 * @return True if the hit box window is resolved, false otherwise
 */
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

/*
 * @brief Set all hurt boxes enabled, it sets all hurt boxes enabled
 * @param bEnabled The enabled
 */
void ABMCharacterBase::SetAllHurtBoxesEnabled(bool bEnabled)
{
	if (!HurtBoxes.Num()) CacheHurtBoxes();

    for (UBMHurtBoxComponent* HB : HurtBoxes)
    {
        if (!HB) continue;
        HB->SetHurtBoxEnabled(bEnabled);
    }
}

/*
 * @brief Try evade incoming hit, it tries to evade the incoming hit
 * @param InInfo The in info
 * @return True if the incoming hit is evaded, false otherwise
 */
bool ABMCharacterBase::TryEvadeIncomingHit(const FBMDamageInfo& InInfo)
{
    (void)InInfo;
    return false; // Ĭ�ϲ�����
}

