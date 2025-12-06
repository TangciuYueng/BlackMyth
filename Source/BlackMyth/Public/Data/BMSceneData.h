#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BMSceneData.generated.h"

USTRUCT(BlueprintType)
struct FBMSceneData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 对应 CSV: MapPath
    // 指向 .umap 文件，例如 /Game/Maps/Forest/Map_Forest.Map_Forest
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene")
    FSoftObjectPath MapPath;

    // 对应 CSV: DisplayName
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene")
    FText DisplayName;
};