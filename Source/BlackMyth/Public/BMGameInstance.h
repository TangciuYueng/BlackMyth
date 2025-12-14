// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BMGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    // 配置：UI蓝图类路径缓存
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMMainWidget> MainMenuClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMPauseMenuWidget> PauseMenuClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMHUDWidget> HUDClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMNotificationWidget> NotificationClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") bool bShowMainMenuOnStartup = true; // Ensure main menu auto-shows at startup

    // 默认游戏地图路径（例如 /Game/Maps/MyDefaultMap 或资产名 MyDefaultMap）
    UPROPERTY(EditDefaultsOnly, Category="Maps") FString DefaultGameMapPath;
};
