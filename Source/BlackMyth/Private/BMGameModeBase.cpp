#include "BMGameModeBase.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/BMPlayerController.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "UI/BMHUDWidget.h"
#include "UI/BMNotificationWidget.h"
#include "BMGameInstance.h"
#include "Character/BMPlayerController.h"

ABMGameModeBase::ABMGameModeBase()
{
    DefaultPawnClass = ABMPlayerCharacter::StaticClass();
    PlayerControllerClass = ABMPlayerController::StaticClass();
}

void ABMGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>();
    UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
    if (!UIManager || !BMGI) return;

    if (BMGI->HUDClass.IsValid())
    {
        UIManager->ShowHUD(BMGI->HUDClass.Get());
    }
    if (BMGI->NotificationClass.IsValid())
    {
        UIManager->ShowNotification(BMGI->NotificationClass.Get());
    }
}


