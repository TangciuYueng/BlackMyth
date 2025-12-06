#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BMItemData.generated.h"

USTRUCT(BlueprintType)
struct FBMItemData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxStack;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Price;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FSoftObjectPath IconPath;
};

