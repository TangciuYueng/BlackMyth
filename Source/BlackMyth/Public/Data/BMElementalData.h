#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BMElementalData.generated.h"

/*
 * @brief Define the FBMElementalData struct, elemental data struct, used to store the elemental data
 * @param FBMElementalData The name of the struct
 * @param FTableRowBase The parent struct
 */
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

    // Poison
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Multiplier")
    float Poison = 1.0f;

    // Lightning
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Multiplier")
    float Lightning = 1.0f;
};
