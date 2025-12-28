// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMPauseMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "System/Save/BMSaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "BMGameInstance.h"
#include "UI/BMMainWidget.h"
#include "Character/Components/BMStatsComponent.h"

#define LOCTEXT_NAMESPACE "BMPauseMenu"

/*
 * @brief Native construct, it native construct
 */
void UBMPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnResumeClicked);
    }
    if (SaveButton)
    {
        SaveButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnSaveClicked);
    }
    if (ReturnToMainButton)
    {
        ReturnToMainButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnReturnToMainClicked);
    }
}

/*
 * @brief Native destruct, it native destruct
 */
void UBMPauseMenuWidget::NativeDestruct()
{
    if (ResumeButton)
    {
        ResumeButton->OnClicked.RemoveAll(this);
    }
    if (SaveButton)
    {
        SaveButton->OnClicked.RemoveAll(this);
    }
    if (ReturnToMainButton)
    {
        ReturnToMainButton->OnClicked.RemoveAll(this);
    }
    Super::NativeDestruct();
}

/*
 * @brief On resume clicked, it on resume clicked
 */
void UBMPauseMenuWidget::OnResumeClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(LOCTEXT("Resume", "Resuming game"));
    }

    // Restore game input and hide pause menu
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeGameOnly GameOnly;
        PC->SetInputMode(GameOnly);
        PC->bShowMouseCursor = false;
    }
    if (UBMUIManagerSubsystem* UI = GetUIManager())
    {
        UI->HidePauseMenu();
    }
}

/*
 * @brief On save clicked, it on save clicked
 */
void UBMPauseMenuWidget::OnSaveClicked()
{
    // Optional: notify via event bus
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(LOCTEXT("PauseSave", "Saving game..."));
    }

    // Get save subsystem from GameInstance and save current game
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UBMSaveGameSubsystem* SaveSubsystem = GI->GetSubsystem<UBMSaveGameSubsystem>())
        {
            const bool bSuccess = SaveSubsystem->SaveCurrentGame();

            if (UBMEventBusSubsystem* Bus2 = GetEventBus())
            {
                if (bSuccess)
                {
                    Bus2->EmitNotify(LOCTEXT("PauseSaveOk", "Game saved"));
                }
                else
                {
                    Bus2->EmitNotify(LOCTEXT("PauseSaveFail", "Save failed"));
                }
            }
        }
    }
}

/*
 * @brief On settings clicked, it on settings clicked
 */
void UBMPauseMenuWidget::OnSettingsClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(LOCTEXT("Settings", "Open Settings"));
    }
}

/*
 * @brief On return to main clicked, it on return to main clicked
 */
void UBMPauseMenuWidget::OnReturnToMainClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(LOCTEXT("ReturnMain", "Returning to main menu..."));
    }
    
    UWorld* World = GetWorld();
    if (!World) return;

    // Hide pause menu first
    if (UBMUIManagerSubsystem* UI = GetUIManager())
    {
        UI->HidePauseMenu();
        UI->HideAllMenus();
    }

    // Stop any background music if needed
    if (UBMGameInstance* BMGI = Cast<UBMGameInstance>(World->GetGameInstance()))
    {
        BMGI->StopLevelMusic();
    }

    // Switch to UI input mode before loading the main menu level
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    // Load the main menu level (emptymap)
    UGameplayStatics::OpenLevel(World, FName(TEXT("emptymap")));
}

/*
 * @brief On skill tree clicked, it on skill tree clicked
 */
void UBMPauseMenuWidget::OnSkillTreeClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(LOCTEXT("SkillTree", "Open Skill Tree"));
    }
}

/*
 * @brief On equipment upgrade clicked, it on equipment upgrade clicked
 */
void UBMPauseMenuWidget::OnEquipmentUpgradeClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(LOCTEXT("EquipmentUpgrade", "Open Equipment Upgrade"));
    }
}

/*
 * @brief Bind event bus, it bind event bus
 * @param EventBus The event bus
 */
void UBMPauseMenuWidget::BindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;
}

/*
 * @brief Unbind event bus, it unbind event bus
 * @param EventBus The event bus
 */
void UBMPauseMenuWidget::UnbindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;
    if (HealthChangedHandle.IsValid())
    {
        EventBus->OnPlayerHealthChanged.Remove(HealthChangedHandle);
        HealthChangedHandle.Reset();
    }
}

#undef LOCTEXT_NAMESPACE
 

