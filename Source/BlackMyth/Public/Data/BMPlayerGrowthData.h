#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BMPlayerGrowthData.generated.h"

USTRUCT(BlueprintType)
struct FBMPlayerGrowthData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 对应 CSV: Level (虽然 RowName 也是 Level，但存个 int 更方便计算)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Growth")
    int32 Level = 1;

    // 对应 CSV: MaxHP
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MaxHP = 100.0f;

    // 对应 CSV: AttackPower
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float AttackPower = 10.0f;

    // 对应 CSV: Defense
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Defense = 0.0f;

    // 对应 CSV: Stamina
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Stamina = 100.0f;

    // 对应 CSV: StaminaRegen
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float StaminaRegen = 5.0f;

    // 对应 CSV: ExpToNext
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Growth")
    float ExpToNext = 100.0f;

    // 对应 CSV: SkillPoints
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
    int32 SkillPoints = 0;

    // 对应 CSV: AttributePoints
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
    int32 AttributePoints = 0;
};
