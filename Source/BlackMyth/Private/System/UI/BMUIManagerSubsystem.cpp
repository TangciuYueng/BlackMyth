// Fill out your copyright notice in the Description page of Project Settings.


#include "System/UI/BMUIManagerSubsystem.h"
#include "UI/BMHUDWidget.h"
#include "UI/BMBossBarBase.h"
#include "UI/BMNotificationWidget.h"
#include "UI/BMPauseMenuWidget.h"
#include "UI/BMMainWidget.h"
#include "UI/BMDeathWidget.h"
#include "Blueprint/UserWidget.h"
#include "System/Event/BMEventBusSubsystem.h"

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
    if (HUD.IsValid())
    {
        UUserWidget* W = HUD.Get();
        if (W && !W->IsInViewport())
        {
            W->AddToViewport();
        }
        return;
    }
    if (UUserWidget* W = CreateAndAdd(HUDClass, World))
    {
        HUD = Cast<UBMHUDWidget>(W);
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
    if (Notification.IsValid())
    {
        UUserWidget* W = Notification.Get();
        if (W && !W->IsInViewport())
        {
            W->AddToViewport();
        }
        return;
    }
    if (UUserWidget* W = CreateAndAdd(NotificationClass, World))
    {
        Notification = Cast<UBMNotificationWidget>(W);
    }
}

void UBMUIManagerSubsystem::ShowPauseMenu(TSubclassOf<UBMPauseMenuWidget> PauseClass)
{
    UWorld* World = GetWorld();
    if (!World) return;
    if (PauseMenu.IsValid())
    {
        UUserWidget* W = PauseMenu.Get();
        if (W && !W->IsInViewport())
        {
            W->AddToViewport();
        }
        return;
    }
    if (UUserWidget* W = CreateAndAdd(PauseClass, World))
    {
        PauseMenu = Cast<UBMPauseMenuWidget>(W);
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

    RemoveIfValid(reinterpret_cast<TWeakObjectPtr<UUserWidget>&>(PauseMenu));
    RemoveIfValid(reinterpret_cast<TWeakObjectPtr<UUserWidget>&>(MainMenu));
    RemoveIfValid(reinterpret_cast<TWeakObjectPtr<UUserWidget>&>(DeathWidget));
}

void UBMUIManagerSubsystem::HidePauseMenu()
{
    if (UUserWidget* W = PauseMenu.Get())
    {
        W->RemoveFromParent();
    }
    PauseMenu = nullptr;
}

void UBMUIManagerSubsystem::ShowDeath(TSubclassOf<UBMDeathWidget> DeathClass)
{
    UWorld* World = GetWorld();
    if (!World) return;
    if (DeathWidget.IsValid())
    {
        UUserWidget* W = DeathWidget.Get();
        if (W && !W->IsInViewport())
        {
            W->AddToViewport();
        }
        return;
    }
    if (UUserWidget* W = CreateAndAdd(DeathClass, World))
    {
        DeathWidget = Cast<UBMDeathWidget>(W);
    }
}

void UBMUIManagerSubsystem::HideDeath()
{
    if (UUserWidget* W = DeathWidget.Get())
    {
        W->RemoveFromParent();
    }
    DeathWidget = nullptr;
}

bool UBMUIManagerSubsystem::IsPauseMenuVisible() const
{
    const UUserWidget* W = PauseMenu.Get();
    return W && W->IsInViewport();
}

void UBMUIManagerSubsystem::HideHUD()
{
    if (UUserWidget* W = HUD.Get())
    {
        W->RemoveFromParent();
    }
    // Keep pointer; HUD could be re-added later without recreation
}

bool UBMUIManagerSubsystem::IsHUDVisible() const
{
    const UUserWidget* W = HUD.Get();
    return W && W->IsInViewport();
}

void UBMUIManagerSubsystem::HideNotification()
{
    if (UUserWidget* W = Notification.Get())
    {
        W->RemoveFromParent();
    }
    // Keep pointer; Notification list persists across toggles
}

bool UBMUIManagerSubsystem::IsNotificationVisible() const
{
    const UUserWidget* W = Notification.Get();
    return W && W->IsInViewport();
}

void UBMUIManagerSubsystem::PushNotificationMessage(const FText& Message)
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (auto* Bus = GI->GetSubsystem<UBMEventBusSubsystem>())
        {
            Bus->EmitNotify(Message);
        }
    }
}

