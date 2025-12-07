#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "BMGameSettings.generated.h"

/**
 *  Config/DefaultGame.ini
 *  Project Settings -> Game -> Black Myth Settings 
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Black Myth Settings"))
class BLACKMYTH_API UBMGameSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> SkillDataTable;

    UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> SceneDataTable;

    UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> ElementDataTable;

    UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> PlayerGrowthTable;

    UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> EnemyDataTable;
};