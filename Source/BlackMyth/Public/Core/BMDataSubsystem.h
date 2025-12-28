#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/BMSkillData.h"        
#include "Data/BMSceneData.h"
#include "Data/BMElementalData.h"
#include "Data/BMPlayerGrowthData.h"
#include "Data/BMEnemyData.h"
#include "Data/BMItemData.h"
#include "BMDataSubsystem.generated.h"

/**
 * @brief Define the UBMDataSubsystem class, data subsystem, used to manage the data
 * @param UBMDataSubsystem The name of the class
 * @param UGameInstanceSubsystem The parent class
 */
UCLASS()
class BLACKMYTH_API UBMDataSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Get skill data from the skill table
    const FBMSkillData* GetSkillData(FName SkillID) const;

    // Get scene data from the scene table
    const FBMSceneData* GetSceneData(FName SceneID) const;

    // Get player growth data from the player growth table
    const FBMPlayerGrowthData* GetPlayerGrowthData(int32 Level) const;

    // Get enemy data from the enemy table
    const FBMEnemyData* GetEnemyData(FName EnemyID) const;

    // Get item data from the item table
    const FBMItemData* GetItemData(FName ItemID) const;

	// Get the item table path for debugging
	FString GetItemTablePathDebug() const;

	// Get the elemental multiplier for the attack and defend elements
    float GetElementalMultiplier(FName AttackElement, FName DefendElement) const;

protected:
    // Cache for the tables
    UPROPERTY()
    UDataTable* SkillTableCache;

    UPROPERTY()
    UDataTable* SceneTableCache;

    UPROPERTY()
    UDataTable* ElementTableCache;

    UPROPERTY()
    UDataTable* PlayerGrowthTableCache;

    UPROPERTY()
    UDataTable* EnemyTableCache;

    UPROPERTY()
    UDataTable* ItemTableCache;

private:
    // Helper function to find a row in a data table
    template <typename T>
    T* FindRow(UDataTable* Table, FName RowName) const;
};