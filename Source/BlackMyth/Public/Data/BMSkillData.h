#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BMSkillData.generated.h"

/*
 * @brief Define the FBMSkillData struct, skill data struct, used to store the skill data
 * @param FBMSkillData The name of the struct
 * @param FTableRowBase The parent struct
 */
USTRUCT(BlueprintType)
struct FBMSkillData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 对应 CSV: SkillName
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    FText SkillName;

    // 对应 CSV: Cooldown
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    float Cooldown = 0.0f;

    // 对应 CSV: DamageMult
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    float DamageMult = 1.0f;

    // 对应 CSV: Cost
    // 技能消耗的资源量（MP 或 Stamina），根据技能类型决定消耗哪种资源
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    float Cost = 0.0f;

    // 对应 CSV: MontagePath
    // 存储格式：/Game/Animations/Montages/AM_CastMagic.AM_CastMagic
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath MontagePath;
};
