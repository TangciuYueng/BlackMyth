#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMStatsComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBMStats, Log, All);


DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnDeathNative, AActor* /*Killer*/);

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMStatsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMStatsComponent();

    virtual void BeginPlay() override;

    float ApplyDamage(FBMDamageInfo& InOutInfo);

    bool IsDead() const { return Stats.HP <= 0.f; }

    bool TryConsumeStamina(float Amount);

    bool TryConsumeMP(float Amount);

    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void AddGameplayTag(FName Tag);
    void RemoveGameplayTag(FName Tag);
    bool HasGameplayTag(FName Tag) const;

    const FBMStatBlock& GetStatBlock() const { return Stats; }
    FBMStatBlock& GetStatBlockMutable() { return Stats; }
    void ReviveToFull(float NewMaxHP);
    void InitializeFromBlock(const FBMStatBlock& In);

public:
    FBMOnDeathNative OnDeathNative;

private:
    UPROPERTY(EditAnywhere, Category = "BM|Stats")
    FBMStatBlock Stats;

    // 耐力每秒恢复速度
    UPROPERTY(EditAnywhere, Category = "BM|Stats", meta = (ClampMin = "0.0"))
    float StaminaRegenPerSec = 10.f;

    bool bDeathBroadcasted = false;
    TSet<FName> Tags;
};
