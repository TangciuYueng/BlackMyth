#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Core/BMTypes.h"
#include "BMSaveData.generated.h"

// 存档数据版本常量
#define BM_SAVE_DATA_VERSION 1

/**
 * Actual save game data class
 */
UCLASS()
class BLACKMYTH_API UBMSaveData : public USaveGame
{
    GENERATED_BODY()

public:
    UBMSaveData();

    /**
     * 验证存档数据有效性
     * 
     * @return 如果数据有效返回 true，否则返回 false
     */
    bool IsValid() const;

    /**
     * 获取存档元信息
     * 
     * @return 包含时间戳、位置等信息的元数据结构
     */
    FBMSaveMeta GetSaveMeta() const;

    // Save metadata
    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    int32 SaveVersion;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    FString SaveSlotName;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    FDateTime Timestamp;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    FName MapName;

    // Player stats
    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    float MaxHP;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    float HP;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    float MaxMP;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    float MP;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    float Stamina;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    float Attack;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    float Defense;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    float MoveSpeed;

    // Player level and experience
    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    int32 PlayerLevel;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    float CurrentXP;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    int32 SkillPoints;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    int32 AttributePoints;

    // Player position and rotation
    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    FVector Location;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    FRotator Rotation;

    // Inventory Data
    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    TArray<FMBInventoryItemSaveData> InventoryItems;

    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    int32 Currency;
};