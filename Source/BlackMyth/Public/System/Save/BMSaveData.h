// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BMSaveData.generated.h"

/**
 * 对应类图: FMBInventoryItemSaveData
 * 用于将 TMap 转换为 TArray 进行存储
 */
USTRUCT(BlueprintType)
struct FMBInventoryItemSaveData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    FName ItemID;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    int32 Quantity;
};

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMSaveData : public USaveGame
{
	GENERATED_BODY()

public:
    UBMSaveData();

    // === 基础元数据 ===
    UPROPERTY(VisibleAnywhere, Category = "Basic")
    FString SaveSlotName;

    UPROPERTY(VisibleAnywhere, Category = "Basic")
    FDateTime Timestamp;

    // === 玩家核心属性 (Player Stats) ===
    UPROPERTY(VisibleAnywhere, Category = "Player Stats")
    float PlayerHP;

    UPROPERTY(VisibleAnywhere, Category = "Player Stats")
    int32 PlayerLevel;

    UPROPERTY(VisibleAnywhere, Category = "Player Stats")
    float CurrentXP;

    UPROPERTY(VisibleAnywhere, Category = "Player Stats")
    int32 SkillPoints;

    UPROPERTY(VisibleAnywhere, Category = "Player Stats")
    FVector Location;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TArray<FMBInventoryItemSaveData> InventoryItems;
};
