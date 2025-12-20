#include "Core/BMDataSubsystem.h"
#include "Config/BMGameSettings.h"

#include "Engine/DataTable.h"

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
        EnemyTableCache = Settings->EnemyDataTable.LoadSynchronous();
        ItemTableCache = Settings->ItemDataTable.LoadSynchronous();
    }

	// C++ fallback: allow running without configuring Project Settings -> Black Myth Settings.
	// Update path here if the asset moves.
	if (!ItemTableCache)
	{
		static const TCHAR* DefaultItemTablePath = TEXT("/Game/Data/Tables/DT_Items.DT_Items");
		ItemTableCache = LoadObject<UDataTable>(nullptr, DefaultItemTablePath);
	}
    
    // Debug Log
    if(!SkillTableCache) UE_LOG(LogTemp, Error, TEXT("BMDataSubsystem: Failed to load Skill Table!"));
	if(!ItemTableCache) UE_LOG(LogTemp, Error, TEXT("BMDataSubsystem: Failed to load Item Table! (Check Project Settings -> Black Myth Settings -> ItemDataTable)"));
}

FString UBMDataSubsystem::GetItemTablePathDebug() const
{
	return GetPathNameSafe(ItemTableCache);
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

const FBMEnemyData* UBMDataSubsystem::GetEnemyData(FName EnemyID) const
{
    return FindRow<FBMEnemyData>(EnemyTableCache, EnemyID);
}

const FBMItemData* UBMDataSubsystem::GetItemData(FName ItemID) const
{
    return FindRow<FBMItemData>(ItemTableCache, ItemID);
}

float UBMDataSubsystem::GetElementalMultiplier(FName AttackElement, FName DefendElement) const
{
    // Attacker 
    const FBMElementalData* Row = FindRow<FBMElementalData>(ElementTableCache, AttackElement);
    if (!Row)
    {
        return 1.0f;
    }

    // Defender
    const UScriptStruct* Struct = FBMElementalData::StaticStruct();

    // Find the property corresponding to the defend element
    const FFloatProperty* FloatProp = FindFProperty<FFloatProperty>(Struct, DefendElement);

    // Return the multiplier value
    if (FloatProp)
    {
        return FloatProp->GetFloatingPointPropertyValue(FloatProp->ContainerPtrToValuePtr<void>(Row));
    }
    // If property not found, log a warning and return default multiplier
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Elemental Multiplier not found for Defense: %s"), *DefendElement.ToString());
        return 1.0f;
    }
}