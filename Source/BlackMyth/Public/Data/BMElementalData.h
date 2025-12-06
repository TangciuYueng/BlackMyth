#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BMElementalData.generated.h"

USTRUCT(BlueprintType)
struct FBMElementalData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // Physical
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Multiplier")
    float Physical = 1.0f;

    // Fire
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Multiplier")
    float Fire = 1.0f;

    // Ice
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Multiplier")
    float Ice = 1.0f;
};
