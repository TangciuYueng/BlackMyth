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

public:
    // Simple persistent data kept across level reloads
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentCoins = 0;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentExp = 0;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentItemCount = 0; // optional aggregate
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentLevel = 1;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentSkillPoints = 0;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentAttributePoints = 0;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") TMap<FName, int32> PersistentItems;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") bool bHasCapturedPersistentData = false;

    // Capture from player state/components before reload
    UFUNCTION(BlueprintCallable, Category="Persistent") void CapturePlayerPersistentData(APlayerController* PC);
    // Restore to player state/components after reload
    UFUNCTION(BlueprintCallable, Category="Persistent") void RestorePlayerPersistentData(APlayerController* PC);
};
