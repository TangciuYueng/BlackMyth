#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BMSceneData.generated.h"

/*
 * @brief Define the FBMSceneData struct, scene data struct, used to store the scene data
 * @param FBMSceneData The name of the struct
 * @param FTableRowBase The parent struct
 */
USTRUCT(BlueprintType)
struct FBMSceneData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // ��Ӧ CSV: MapPath
    // ָ�� .umap �ļ������� /Game/Maps/Forest/Map_Forest.Map_Forest
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene")
    FSoftObjectPath MapPath;

    // ��Ӧ CSV: DisplayName
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene")
    FText DisplayName;
};