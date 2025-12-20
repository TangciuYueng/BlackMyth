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

UCLASS()
class BLACKMYTH_API UBMDataSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // API to get data from tables
    const FBMSkillData* GetSkillData(FName SkillID) const;

    const FBMSceneData* GetSceneData(FName SceneID) const;

    const FBMPlayerGrowthData* GetPlayerGrowthData(int32 Level) const;

    const FBMEnemyData* GetEnemyData(FName EnemyID) const;

    const FBMItemData* GetItemData(FName ItemID) const;

	FString GetItemTablePathDebug() const;

    float GetElementalMultiplier(FName AttackElement, FName DefendElement) const;

protected:
    // Caches for data tables
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