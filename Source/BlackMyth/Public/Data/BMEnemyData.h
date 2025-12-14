#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BMEnemyData.generated.h"

/**
 * 敌人数据结构体
 * 对应 DataTable: DT_Enemies
 */
USTRUCT(BlueprintType)
struct FBMEnemyData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MaxHP = 100.0f; // 给个默认值是个好习惯

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float AttackPower = 10.0f;

    // 骨骼模型路径 (Skeletal Mesh)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath MeshPath;

    // 动画路径 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath AnimPath;

    // 行为树路径
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    FSoftObjectPath BehaviorTreePath;
};
