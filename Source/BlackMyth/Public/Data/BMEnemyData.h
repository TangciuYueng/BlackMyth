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
    // ===== 基础属性 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MaxHP = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float AttackPower = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float Defense = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MoveSpeed = 600.0f;

    // ===== AI 参数 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    float AggroRange = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    float PatrolRadius = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    float PatrolSpeed = 220.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    float ChaseSpeed = 420.f;

    // ===== 闪避参数 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float DodgeDistance = 420.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float DodgeOnHitChance = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float DodgeCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float DodgePlayRate = 1.0f;

    // ===== 掉落参数 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    int32 CurrencyDropMin = 200;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    int32 CurrencyDropMax = 300;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    float ExpDropMin = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    float ExpDropMax = 300.0f;

    // ===== 资产路径 =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath MeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath AnimIdlePath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath AnimWalkPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath AnimRunPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath AnimHitLightPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath AnimHitHeavyPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath AnimDeathPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    FSoftObjectPath AnimDodgePath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    FSoftObjectPath BehaviorTreePath;
};
