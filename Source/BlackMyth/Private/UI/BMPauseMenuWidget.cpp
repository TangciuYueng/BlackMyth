// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMPauseMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "BMGameInstance.h"
#include "UI/BMMainWidget.h"

void UBMPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnResumeClicked);
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
}

void UBMPauseMenuWidget::NativeDestruct()
{
    if (ResumeButton)
    {
        ResumeButton->OnClicked.RemoveAll(this);
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
        Bus->EmitNotify(NSLOCTEXT("BM", "Resume", "Resuming game"));
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

void UBMPauseMenuWidget::OnSettingsClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(NSLOCTEXT("BM", "Settings", "Open Settings"));
    }
}

void UBMPauseMenuWidget::OnReturnToMainClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(NSLOCTEXT("BM", "ReturnMain", "Return to main menu"));
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
        Bus->EmitNotify(NSLOCTEXT("BM", "SkillTree", "Open Skill Tree"));
    }
}

void UBMPauseMenuWidget::OnEquipmentUpgradeClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(NSLOCTEXT("BM", "EquipmentUpgrade", "Open Equipment Upgrade"));
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
    if (!ManaChangedHandle.IsValid())
    {
        ManaChangedHandle = EventBus->OnPlayerManaChanged.AddLambda([this](float Normalized)
        {
            UpdateManaText(Normalized);
        });
    }
    // Initial refresh if needed
    RefreshResourceText();
}

void UBMPauseMenuWidget::UnbindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;
    if (HealthChangedHandle.IsValid())
    {
        EventBus->OnPlayerHealthChanged.Remove(HealthChangedHandle);
        HealthChangedHandle.Reset();
    }
    if (ManaChangedHandle.IsValid())
    {
        EventBus->OnPlayerManaChanged.Remove(ManaChangedHandle);
        ManaChangedHandle.Reset();
    }
}

void UBMPauseMenuWidget::UpdateHealthText(float Normalized)
{
    if (!HealthText) return;
    // Assume max 500 for demo; in real case bind to player state/attributes
    const int32 MaxHealth = 500;
    const int32 CurHealth = FMath::RoundToInt(Normalized * MaxHealth);
    const FText Txt = FText::Format(NSLOCTEXT("BM", "PauseHealthFmt", "生命值：{0}/{1}"), CurHealth, MaxHealth);
    HealthText->SetText(Txt);
}

void UBMPauseMenuWidget::UpdateManaText(float Normalized)
{
    if (!ManaText) return;
    const int32 MaxMana = 100;
    const int32 CurMana = FMath::RoundToInt(Normalized * MaxMana);
    const FText Txt = FText::Format(NSLOCTEXT("BM", "PauseManaFmt", "法力值：{0}/{1}"), CurMana, MaxMana);
    ManaText->SetText(Txt);
}

void UBMPauseMenuWidget::RefreshResourceText()
{
    if (!ResourceText) return;
    // Placeholder static values; should be replaced with real inventory/skill points
    const int32 Iron = 25;
    const int32 SkillPoints = 3;
    const FText Txt = FText::Format(NSLOCTEXT("BM", "PauseResFmt", "精铁：{0} 技能点：{1}"), Iron, SkillPoints);
    ResourceText->SetText(Txt);
}

