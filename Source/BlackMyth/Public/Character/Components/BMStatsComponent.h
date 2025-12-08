#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "BMStatsComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBMStats, Log, All);

// 纯C++死亡事件（不靠蓝图）
DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnDeathNative, AActor* /*Killer*/);

UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMStatsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMStatsComponent();

    // 统一使用 FBMDamageInfo 作为伤害载体（会回填 DamageValue = 最终生效伤害）
    float ApplyDamage(FBMDamageInfo& InOutInfo);

    bool IsDead() const { return Stats.HP <= 0.f; }

    bool TryConsumeStamina(float Amount);

    void AddGameplayTag(FName Tag);
    void RemoveGameplayTag(FName Tag);
    bool HasGameplayTag(FName Tag) const;

    // 便于角色/系统读取
    const FBMStatBlock& GetStatBlock() const { return Stats; }
    FBMStatBlock& GetStatBlockMutable() { return Stats; }



public:
    UPROPERTY(EditAnywhere, Category = "BM|Stats")
    FBMStatBlock Stats;

    FBMOnDeathNative OnDeathNative;

private:
    bool bDeathBroadcasted = false;
    TSet<FName> Tags;
};
