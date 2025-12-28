#include "BMGameModeBase.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/BMPlayerController.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "UI/BMHUDWidget.h"
#include "UI/BMGameHUD.h"
#include "UI/BMNotificationWidget.h"
#include "UI/BMBossBarBase.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "BMGameInstance.h"
#include "Character/BMPlayerController.h"
#include "Kismet/GameplayStatics.h"

ABMGameModeBase::ABMGameModeBase()
{
    DefaultPawnClass = ABMPlayerCharacter::StaticClass();
    PlayerControllerClass = ABMPlayerController::StaticClass();
    HUDClass = ABMGameHUD::StaticClass();
}

void ABMGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("BMGameModeBase::BeginPlay: GameInstance is null"));
        return;
    }

    // Diagnostics: log current map and GI class
    const UWorld* World = GetWorld();
    const FString CurrentLevelName = World ? UGameplayStatics::GetCurrentLevelName(const_cast<UObject*>(static_cast<const UObject*>(World)), true) : TEXT("<no world>");
    UE_LOG(LogTemp, Log, TEXT("BMGameModeBase::BeginPlay on map '%s', GI class: %s"), *CurrentLevelName, *GI->GetClass()->GetName());

    UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>();
    UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
    if (!UIManager)
    {
        UE_LOG(LogTemp, Error, TEXT("BMGameModeBase::BeginPlay: UIManager subsystem not found"));
        return;
    }
    if (!BMGI)
    {
        UE_LOG(LogTemp, Error, TEXT("BMGameModeBase::BeginPlay: GameInstance is not UBMGameInstance. Set Project Settings -> Maps & Modes -> Game Instance Class"));
        return;
    }

    // HUD
    if (BMGI->HUDClass.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("BMGameModeBase: ShowHUD with GI HUDClass %s"), *BMGI->HUDClass->GetName());
        UIManager->ShowHUD(BMGI->HUDClass.Get());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("BMGameModeBase: HUDClass not set in GameInstance. Trying fallback /Game/UI/WBP_HUD.WBP_HUD_C"));
        if (UClass* FallbackHUD = LoadClass<UBMHUDWidget>(nullptr, TEXT("/Game/UI/WBP_HUD.WBP_HUD_C")))
        {
            UE_LOG(LogTemp, Log, TEXT("BMGameModeBase: ShowHUD with fallback WBP_HUD"));
            UIManager->ShowHUD(FallbackHUD);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BMGameModeBase: Fallback HUD class not found. Set HUDClass in GameInstance."));
        }
    }

    // Notification
    if (BMGI->NotificationClass.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("BMGameModeBase: ShowNotification with GI NotificationClass %s"), *BMGI->NotificationClass->GetName());
        UIManager->ShowNotification(BMGI->NotificationClass.Get());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("BMGameModeBase: NotificationClass not set in GameInstance. Trying fallback /Game/UI/WBP_Notification.WBP_Notification_C"));
        if (UClass* FallbackNotif = LoadClass<UBMNotificationWidget>(nullptr, TEXT("/Game/UI/WBP_Notification.WBP_Notification_C")))
        {
            UE_LOG(LogTemp, Log, TEXT("BMGameModeBase: ShowNotification with fallback WBP_Notification"));
            UIManager->ShowNotification(FallbackNotif);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BMGameModeBase: Fallback Notification class not found. Set NotificationClass in GameInstance."));
        }
    }

    // Boss bar: only show on boss map STZD_Demo_01
    if (CurrentLevelName == TEXT("STZD_Demo_01"))
    {
        UE_LOG(LogTemp, Log, TEXT("BMGameModeBase: Boss map detected (%s). Attempting to show BossBar."), *CurrentLevelName);
        if (UClass* BossBarClass = LoadClass<UBMBossBarBase>(nullptr, TEXT("/Game/UI/WBP_BossBar.WBP_BossBar_C")))
        {
            UIManager->ShowBossBar(BossBarClass);
            UE_LOG(LogTemp, Log, TEXT("BMGameModeBase: BossBar shown."));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BMGameModeBase: BossBar widget class not found at /Game/UI/WBP_BossBar.WBP_BossBar_C"));
        }
    }

}



