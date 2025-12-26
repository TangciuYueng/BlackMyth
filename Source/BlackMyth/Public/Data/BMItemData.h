#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Core/BMTypes.h"
#include "BMItemData.generated.h"

USTRUCT(BlueprintType)
struct FBMItemData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 基础属性
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Basic")
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Basic")
    int32 MaxStack = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Basic")
    float Price = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Basic")
    FSoftObjectPath IconPath;

    // 属性加成
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
    float MaxHp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
    float Hp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stats")
    float AttackPower = 0.0f;

    // 额外功能
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Effect")
    EBMItemFunction AdditionalFunction = EBMItemFunction::None;
};

