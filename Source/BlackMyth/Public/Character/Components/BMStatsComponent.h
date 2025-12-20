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

    float ApplyDamage(FBMDamageInfo& InOutInfo);

    bool IsDead() const { return Stats.HP <= 0.f; }

    bool TryConsumeStamina(float Amount);

    /**
     * 尝试消耗魔法值（MP）
     * 
     * @param Amount 要消耗的 MP 数量（必须大于0）
     * @return 如果当前 MP 足够并成功消耗返回 true，否则返回 false
     */
    bool TryConsumeMP(float Amount);

    void AddGameplayTag(FName Tag);
    void RemoveGameplayTag(FName Tag);
    bool HasGameplayTag(FName Tag) const;

    const FBMStatBlock& GetStatBlock() const { return Stats; }
    FBMStatBlock& GetStatBlockMutable() { return Stats; }

    void InitializeFromBlock(const FBMStatBlock& In);

public:
    UPROPERTY(EditAnywhere, Category = "BM|Stats")
    FBMStatBlock Stats;

    FBMOnDeathNative OnDeathNative;

private:
    bool bDeathBroadcasted = false;
    TSet<FName> Tags;
};
