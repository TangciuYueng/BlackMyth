// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BMUIManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void ShowHUD(TSubclassOf<class UBMHUDWidget> HUDClass);

    UFUNCTION(BlueprintCallable)
    void HideHUD();

    UFUNCTION(BlueprintCallable)
    bool IsHUDVisible() const;

    UFUNCTION(BlueprintCallable)
    void ShowBossBar(TSubclassOf<class UBMBossBarBase> BossBarClass);

    UFUNCTION(BlueprintCallable)
    void ShowNotification(TSubclassOf<class UBMNotificationWidget> NotificationClass);

    UFUNCTION(BlueprintCallable)
    void HideNotification();

    UFUNCTION(BlueprintCallable)
    bool IsNotificationVisible() const;

    // Convenience to push a notification via EventBus
    UFUNCTION(BlueprintCallable)
    void PushNotificationMessage(const FText& Message);

    UFUNCTION(BlueprintCallable)
    void ShowPauseMenu(TSubclassOf<class UBMPauseMenuWidget> PauseClass);

    UFUNCTION(BlueprintCallable)
    void HidePauseMenu();

    UFUNCTION(BlueprintCallable)
    bool IsPauseMenuVisible() const;

    UFUNCTION(BlueprintCallable)
    void ShowMainMenu(TSubclassOf<class UBMMainWidget> MainClass);

    UFUNCTION(BlueprintCallable)
    void HideMainMenu();

    UFUNCTION(BlueprintCallable)
    bool IsMainMenuVisible() const;

    UFUNCTION(BlueprintCallable)
    void HideAllMenus();

    UFUNCTION(BlueprintCallable)
    void ShowDeath(TSubclassOf<class UBMDeathWidget> DeathClass);
    
    UFUNCTION(BlueprintCallable)
    void HideDeath();

    UFUNCTION(BlueprintCallable)
    void ShowSaveLoadMenu(TSubclassOf<class UBMSaveLoadMenuWidget> SaveLoadClass);

    UFUNCTION(BlueprintCallable)
    void HideSaveLoadMenu();

    UFUNCTION(BlueprintCallable)
    bool IsSaveLoadMenuVisible() const;

private:
    TWeakObjectPtr<class UBMHUDWidget> HUD;
    TWeakObjectPtr<class UBMBossBarBase> BossBar;
    TWeakObjectPtr<class UBMNotificationWidget> Notification;
    TWeakObjectPtr<class UBMPauseMenuWidget> PauseMenu;
    TWeakObjectPtr<class UBMMainWidget> MainMenu;
    TWeakObjectPtr<class UBMDeathWidget> DeathWidget;
    TWeakObjectPtr<class UBMSaveLoadMenuWidget> SaveLoadMenu;
};
