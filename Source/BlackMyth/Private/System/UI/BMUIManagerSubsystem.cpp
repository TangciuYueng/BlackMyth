// Fill out your copyright notice in the Description page of Project Settings.


#include "System/UI/BMUIManagerSubsystem.h"
#include "UI/BMHUDWidget.h"
#include "UI/BMBossBarBase.h"
#include "UI/BMNotificationWidget.h"
#include "UI/BMPauseMenuWidget.h"
#include "UI/BMMainWidget.h"
#include "Blueprint/UserWidget.h"

namespace
{
    static UUserWidget* CreateAndAdd(TSubclassOf<UUserWidget> Class, UWorld* World)
    {
        if (!Class || !World) return nullptr;
        UUserWidget* Widget = CreateWidget<UUserWidget>(World, Class);
        if (Widget)
        {
            Widget->AddToViewport();
        }
        return Widget;
    }
}

void UBMUIManagerSubsystem::ShowHUD(TSubclassOf<UBMHUDWidget> HUDClass)
{
    UWorld* World = GetWorld();
    if (!World) return;
    if (!HUD.IsValid())
    {
        if (UUserWidget* W = CreateAndAdd(HUDClass, World))
        {
            HUD = Cast<UBMHUDWidget>(W);
        }
    }
}

void UBMUIManagerSubsystem::ShowBossBar(TSubclassOf<UBMBossBarBase> BossBarClass)
{
    UWorld* World = GetWorld();
    if (!World) return;
    if (!BossBar.IsValid())
    {
        if (UUserWidget* W = CreateAndAdd(BossBarClass, World))
        {
            BossBar = Cast<UBMBossBarBase>(W);
        }
    }
}

void UBMUIManagerSubsystem::ShowNotification(TSubclassOf<UBMNotificationWidget> NotificationClass)
{
    UWorld* World = GetWorld();
    if (!World) return;
    if (!Notification.IsValid())
    {
        if (UUserWidget* W = CreateAndAdd(NotificationClass, World))
        {
            Notification = Cast<UBMNotificationWidget>(W);
        }
    }
}

void UBMUIManagerSubsystem::ShowPauseMenu(TSubclassOf<UBMPauseMenuWidget> PauseClass)
{
    UWorld* World = GetWorld();
    if (!World) return;
    if (!PauseMenu.IsValid())
    {
        if (UUserWidget* W = CreateAndAdd(PauseClass, World))
        {
            PauseMenu = Cast<UBMPauseMenuWidget>(W);
        }
    }
}

void UBMUIManagerSubsystem::ShowMainMenu(TSubclassOf<UBMMainWidget> MainClass)
{
    UWorld* World = GetWorld();
    if (!World) return;
    if (!MainMenu.IsValid())
    {
        if (UUserWidget* W = CreateAndAdd(MainClass, World))
        {
            MainMenu = Cast<UBMMainWidget>(W);
        }
    }
}

void UBMUIManagerSubsystem::HideAllMenus()
{
    auto RemoveIfValid = [](TWeakObjectPtr<UUserWidget>& WPtr)
    {
        if (UUserWidget* W = WPtr.Get())
        {
            W->RemoveFromParent();
        }
        WPtr = nullptr;
    };

    TWeakObjectPtr<UUserWidget> Pause(PauseMenu.Get());
    TWeakObjectPtr<UUserWidget> Main(MainMenu.Get());

    RemoveIfValid(Pause);
    RemoveIfValid(Main);
}

