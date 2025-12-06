#include "Core/BMDataSubsystem.h"
#include "Config/BMGameSettings.h"

void UBMDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UBMGameSettings* Settings = GetDefault<UBMGameSettings>();

    if (Settings)
    {
        SkillTableCache = Settings->SkillDataTable.LoadSynchronous();
        SceneTableCache = Settings->SceneDataTable.LoadSynchronous();
        ElementTableCache = Settings->ElementDataTable.LoadSynchronous();
        PlayerGrowthTableCache = Settings->PlayerGrowthTable.LoadSynchronous();
    }
    
    // Debug Log
    if(!SkillTableCache) UE_LOG(LogTemp, Error, TEXT("BMDataSubsystem: Failed to load Skill Table!"));
}

template <typename T>
T* UBMDataSubsystem::FindRow(UDataTable* Table, FName RowName) const
{
    if (!Table) return nullptr;
    static const FString ContextString(TEXT("BMDataSubsystem Lookup"));
    return Table->FindRow<T>(RowName, ContextString);
}

const FBMSkillData* UBMDataSubsystem::GetSkillData(FName SkillID) const
{
    return FindRow<FBMSkillData>(SkillTableCache, SkillID);
}

const FBMSceneData* UBMDataSubsystem::GetSceneData(FName SceneID) const
{
    return FindRow<FBMSceneData>(SceneTableCache, SceneID);
}

const FBMPlayerGrowthData* UBMDataSubsystem::GetPlayerGrowthData(int32 Level) const
{
    return FindRow<FBMPlayerGrowthData>(PlayerGrowthTableCache, *FString::FromInt(Level));
}

float UBMDataSubsystem::GetElementalMultiplier(FName AttackElement, FName DefendElement) const
{
    const FBMElementalData* Row = FindRow<FBMElementalData>(ElementTableCache, AttackElement);
    if (!Row) return 1.0f;

    if (DefendElement == FName("Physical")) return Row->Physical;
    if (DefendElement == FName("Fire")) return Row->Fire;
    if (DefendElement == FName("Ice")) return Row->Ice;

    return 1.0f;
}