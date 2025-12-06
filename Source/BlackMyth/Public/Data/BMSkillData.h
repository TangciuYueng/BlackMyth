#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BMSkillData.generated.h"

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

    // 对应 CSV: MontagePath
    // 存储格式如: /Game/Animations/Montages/AM_CastMagic.AM_CastMagic
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath MontagePath;
};