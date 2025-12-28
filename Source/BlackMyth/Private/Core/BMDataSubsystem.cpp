#include "Core/BMDataSubsystem.h"
#include "Config/BMGameSettings.h"

#include "Engine/DataTable.h"

/*
 * @brief Initialize, it initializes the data subsystem
 * @param Collection The collection
 */
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

/*
 * @brief Get the item table path debug, it gets the item table path debug
 * @return The item table path debug
 */
FString UBMDataSubsystem::GetItemTablePathDebug() const
{
	return GetPathNameSafe(ItemTableCache);
}

/*
 * @brief Find row, it finds the row
 * @param Table The table
 * @param RowName The row name
 * @return The row
 */
template <typename T>
T* UBMDataSubsystem::FindRow(UDataTable* Table, FName RowName) const
{
    if (!Table) return nullptr;
    static const FString ContextString(TEXT("BMDataSubsystem Lookup"));
    return Table->FindRow<T>(RowName, ContextString);
}

/*
 * @brief Get the skill data, it gets the skill data
 * @param SkillID The skill id
 * @return The skill data
 */
const FBMSkillData* UBMDataSubsystem::GetSkillData(FName SkillID) const
{
    return FindRow<FBMSkillData>(SkillTableCache, SkillID);
}

/*
 * @brief Get the scene data, it gets the scene data
 * @param SceneID The scene id
 * @return The scene data
 */
const FBMSceneData* UBMDataSubsystem::GetSceneData(FName SceneID) const
{
    return FindRow<FBMSceneData>(SceneTableCache, SceneID);
}

/*
 * @brief Get the player growth data, it gets the player growth data
 * @param Level The level
 * @return The player growth data
 */
const FBMPlayerGrowthData* UBMDataSubsystem::GetPlayerGrowthData(int32 Level) const
{
    return FindRow<FBMPlayerGrowthData>(PlayerGrowthTableCache, *FString::FromInt(Level));
}

/*
 * @brief Get the enemy data, it gets the enemy data
 * @param EnemyID The enemy id
 * @return The enemy data
 */
const FBMEnemyData* UBMDataSubsystem::GetEnemyData(FName EnemyID) const
{
    return FindRow<FBMEnemyData>(EnemyTableCache, EnemyID);
}

/*
 * @brief Get the item data, it gets the item data
 * @param ItemID The item id
 * @return The item data
 */
const FBMItemData* UBMDataSubsystem::GetItemData(FName ItemID) const
{
    return FindRow<FBMItemData>(ItemTableCache, ItemID);
}

/*
 * @brief Get the elemental multiplier, it gets the elemental multiplier
 * @param AttackElement The attack element
 * @param DefendElement The defend element
 * @return The elemental multiplier
 */
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