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
    if (SkillTreeButton)
    {
        SkillTreeButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnSkillTreeClicked);
    }
    if (EquipmentUpgradeButton)
    {
        EquipmentUpgradeButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnEquipmentUpgradeClicked);
    }
    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnSettingsClicked);
    }
    if (ReturnToMainButton)
    {
        ReturnToMainButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnReturnToMainClicked);
    }

    // Proactive refresh: read current player HP/MaxHP and update text immediately
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        APawn* Pawn = PC->GetPawn();
        if (Pawn)
        {
            if (UBMStatsComponent* Stats = Pawn->FindComponentByClass<UBMStatsComponent>())
            {
                const float MaxHP = FMath::Max(1.f, Stats->GetStatBlockMutable().MaxHP);
                const float HP = FMath::Clamp(Stats->GetStatBlockMutable().HP, 0.f, MaxHP);
                const float Normalized = HP / MaxHP;
                UpdateHealthText(Normalized);
            }
        }
    }
}

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
    if (SkillTreeButton)
    {
        SkillTreeButton->OnClicked.RemoveAll(this);
    }
    if (EquipmentUpgradeButton)
    {
        EquipmentUpgradeButton->OnClicked.RemoveAll(this);
    }
    if (SettingsButton)
    {
        SettingsButton->OnClicked.RemoveAll(this);
    }
    if (ReturnToMainButton)
    {
        ReturnToMainButton->OnClicked.RemoveAll(this);
    }
    Super::NativeDestruct();
}

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

void UBMPauseMenuWidget::OnSettingsClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(LOCTEXT("Settings", "Open Settings"));
    }
}

void UBMPauseMenuWidget::OnReturnToMainClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(LOCTEXT("ReturnMain", "Return to main menu"));
    }
    // Hide pause and show main menu
    UWorld* World = GetWorld();
    if (!World) return;
    if (UBMUIManagerSubsystem* UI = GetUIManager())
    {
        UI->HidePauseMenu();

        // Resolve main menu class from GameInstance or fallback paths
        UBMGameInstance* BMGI = Cast<UBMGameInstance>(World->GetGameInstance());
        TSubclassOf<UBMMainWidget> MainClass = nullptr;
        if (BMGI && BMGI->MainMenuClass.IsValid())
        {
            MainClass = BMGI->MainMenuClass.Get();
        }
        if (!MainClass)
        {
            MainClass = LoadClass<UBMMainWidget>(nullptr, TEXT("/Game/UI/WBP_MainMenu.WBP_MainMenu_C"));
        }
        if (!MainClass)
        {
            MainClass = LoadClass<UBMMainWidget>(nullptr, TEXT("/Game/BlackMyth/UI/WBP_MainMenu.WBP_MainMenu_C"));
        }
        if (MainClass)
        {
            UI->ShowMainMenu(MainClass);
        }
    }

    // Switch to UI only for main menu interaction
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
}

void UBMPauseMenuWidget::OnSkillTreeClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(LOCTEXT("SkillTree", "Open Skill Tree"));
    }
}

void UBMPauseMenuWidget::OnEquipmentUpgradeClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(LOCTEXT("EquipmentUpgrade", "Open Equipment Upgrade"));
    }
}

void UBMPauseMenuWidget::BindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;
    if (!HealthChangedHandle.IsValid())
    {
        HealthChangedHandle = EventBus->OnPlayerHealthChanged.AddLambda([this](float Normalized)
        {
            UpdateHealthText(Normalized);
        });
    }
    // Initial refresh if needed (status texts can be filled by future events)
}

void UBMPauseMenuWidget::UnbindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;
    if (HealthChangedHandle.IsValid())
    {
        EventBus->OnPlayerHealthChanged.Remove(HealthChangedHandle);
        HealthChangedHandle.Reset();
    }
}

void UBMPauseMenuWidget::UpdateHealthText(float Normalized)
{
    if (!HealthText) return;
    // Assume max 500 for demo; in real case bind to player state/attributes
    const int32 MaxHealth = 500;
    const int32 CurHealth = FMath::RoundToInt(Normalized * MaxHealth);
    const FText Txt = FText::Format(LOCTEXT("PauseHealthFmt", "{0}/{1}"), CurHealth, MaxHealth);
    HealthText->SetText(Txt);
}

#undef LOCTEXT_NAMESPACE
 

