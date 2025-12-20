#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BMPlayerGrowthData.generated.h"

USTRUCT(BlueprintType)
struct FBMPlayerGrowthData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // Level as INT for easy calculations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Growth")
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MaxHP = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float AttackPower = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Defense = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float StaminaRegen = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MaxMP = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Growth")
    float ExpToNext = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
    int32 SkillPoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
    int32 AttributePoints = 0;
};
