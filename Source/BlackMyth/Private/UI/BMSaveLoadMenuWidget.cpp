// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BMSaveLoadMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "System/Save/BMSaveGameSubsystem.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "UI/BMMainWidget.h"
#include "BMGameInstance.h"
#include "Core/BMTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void UBMSaveLoadMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events
    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UBMSaveLoadMenuWidget::OnBackClicked);
    }

    if (Slot1)
    {
        Slot1->OnClicked.AddDynamic(this, &UBMSaveLoadMenuWidget::OnSlot1Clicked);
    }

    // Refresh save slot display
    RefreshSaveSlotDisplay();
}

void UBMSaveLoadMenuWidget::NativeDestruct()
{
    // Unbind button events
    if (BackButton)
    {
        BackButton->OnClicked.RemoveAll(this);
    }

    if (Slot1)
    {
        Slot1->OnClicked.RemoveAll(this);
    }

    Super::NativeDestruct();
}

void UBMSaveLoadMenuWidget::OnBackClicked()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Get current level name once
    FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World, true);
    bool bIsMainMenuLevel = CurrentLevelName.Contains(TEXT("emptymap")) || CurrentLevelName.Contains(TEXT("MainMenu"));
    
    // Hide the SaveLoad menu using UIManager
    if (UBMUIManagerSubsystem* UIManager = GetUIManager())
    {
        UIManager->HideSaveLoadMenu();
        
        // Check if we're in main menu scene, if so, restore main menu
        if (bIsMainMenuLevel)
        {
            // We're in main menu, restore main menu
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
                MainClass = LoadClass<UBMMainWidget>(nullptr, TEXT("/Game//UI/WBP_MainMenu.WBP_MainMenu_C"));
            }
            if (MainClass)
            {
                UIManager->ShowMainMenu(MainClass);
            }
        }
    }
    else
    {
        // Fallback: just remove from parent
        RemoveFromParent();
    }

    // If we're in main menu, restore UI input mode
    // If we're in game, restore game input mode
    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        if (bIsMainMenuLevel)
        {
            // We're in main menu, keep UI mode
            FInputModeUIOnly InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
        }
        else
        {
            // We're in game, restore game mode
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
        }
    }
}

void UBMSaveLoadMenuWidget::OnSlot1Clicked()
{
    LoadGameIndex(1);
}

void UBMSaveLoadMenuWidget::OnSlot2Clicked()
{
    LoadGameIndex(2);
}

void UBMSaveLoadMenuWidget::OnSlot3Clicked()
{
    LoadGameIndex(3);
}

void UBMSaveLoadMenuWidget::OnSlot4Clicked()
{
    LoadGameIndex(4);
}

void UBMSaveLoadMenuWidget::LoadGameIndex(int32 SlotIndex)
{
    UWorld* World = GetWorld();
    if (!World) return;

    UGameInstance* GI = World->GetGameInstance();
    if (!GI) return;

    UBMSaveGameSubsystem* SaveSubsystem = GI->GetSubsystem<UBMSaveGameSubsystem>();
    if (!SaveSubsystem) return;

    // Check if save exists
    if (!SaveSubsystem->DoesSaveExist(SlotIndex))
    {
        // No save exists, show notification
        if (UBMEventBusSubsystem* EventBus = GetEventBus())
        {
            EventBus->EmitNotify(NSLOCTEXT("BMSaveLoad", "NoSave", "No save file found in this slot."));
        }
        return;
    }

    // Check if we're in main menu
    FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World, true);
    bool bIsMainMenuLevel = CurrentLevelName.Contains(TEXT("emptymap")) || CurrentLevelName.Contains(TEXT("MainMenu"));

    bool bSuccess = false;

    if (bIsMainMenuLevel)
    {
        // From main menu: need to load the map first, then restore save data
        bSuccess = SaveSubsystem->LoadGameFromMainMenu(SlotIndex);
        
        if (bSuccess)
        {
            // Hide the menu immediately (map will change)
            RemoveFromParent();
            
            // Hide all menus
            if (UBMUIManagerSubsystem* UIManager = GetUIManager())
            {
                UIManager->HideAllMenus();
            }
        }
        else
        {
            // Show error notification
            if (UBMEventBusSubsystem* EventBus = GetEventBus())
            {
                EventBus->EmitNotify(NSLOCTEXT("BMSaveLoad", "LoadFailed", "Failed to load game."));
            }
        }
    }
    else
    {
        // In game: just load and restore position
        bSuccess = SaveSubsystem->LoadGame(SlotIndex);
        
        if (bSuccess)
        {
            // Show success notification
            if (UBMEventBusSubsystem* EventBus = GetEventBus())
            {
                EventBus->EmitNotify(NSLOCTEXT("BMSaveLoad", "LoadSuccess", "Game loaded successfully!"));
            }

            // Hide the menu and switch to game input mode
            RemoveFromParent();
            
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                FInputModeGameOnly InputMode;
                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = false;
            }

            // Hide all menus
            if (UBMUIManagerSubsystem* UIManager = GetUIManager())
            {
                UIManager->HideAllMenus();
            }
        }
        else
        {
            // Show error notification
            if (UBMEventBusSubsystem* EventBus = GetEventBus())
            {
                EventBus->EmitNotify(NSLOCTEXT("BMSaveLoad", "LoadFailed", "Failed to load game."));
            }
        }
    }
}

void UBMSaveLoadMenuWidget::RefreshSaveSlotDisplay()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UBMSaveGameSubsystem* SaveSubsystem = GI->GetSubsystem<UBMSaveGameSubsystem>())
            {
                // Get all save slots (we only need slots 1-4)
                TArray<FBMSaveSlotInfo> SaveSlots = SaveSubsystem->GetAllSaveSlots(10);

                // Update button text for each slot
                UpdateSlotButton(Slot1, 1, SaveSlots);
            }
        }
    }
}

void UBMSaveLoadMenuWidget::UpdateSlotButton(UButton* Button, int32 SlotIndex, const TArray<FBMSaveSlotInfo>& SaveSlots)
{
    if (!Button)
    {
        return;
    }

    // Find the slot info for this slot index
    const FBMSaveSlotInfo* SlotInfo = SaveSlots.FindByPredicate([SlotIndex](const FBMSaveSlotInfo& Info)
    {
        return Info.SlotNumber == SlotIndex;
    });

    if (!SlotInfo)
    {
        // Slot not found, set default empty text
        SetButtonText(Button, FText::Format(NSLOCTEXT("BMSaveLoad", "EmptySlot", "Slot {0}\nEmpty"), SlotIndex));
        return;
    }

    if (SlotInfo->bExists)
    {
        // Save exists, format the display text with timestamp and map name
        FString TimeString = SlotInfo->Meta.Timestamp.ToString(TEXT("%Y-%m-%d %H:%M"));
        FString MapNameString = SlotInfo->Meta.MapName.ToString();
        
        FText DisplayText = FText::Format(
            NSLOCTEXT("BMSaveLoad", "SaveSlot", "Slot {0}\n{1}\n{2}"),
            SlotIndex,
            FText::FromString(TimeString),
            FText::FromString(MapNameString)
        );
        
        SetButtonText(Button, DisplayText);
    }
    else
    {
        // No save exists
        SetButtonText(Button, FText::Format(NSLOCTEXT("BMSaveLoad", "EmptySlot", "Slot {0}\nEmpty"), SlotIndex));
    }
}

void UBMSaveLoadMenuWidget::SetButtonText(UButton* Button, const FText& Text)
{
    if (!Button)
    {
        return;
    }

    // Try to find a TextBlock child widget to update
    // This is a common pattern in Unreal where buttons contain TextBlocks
    TArray<UWidget*> AllChildren;
    AllChildren = Button->GetAllChildren();

    for (UWidget* Child : AllChildren)
    {
        if (UTextBlock* TextBlock = Cast<UTextBlock>(Child))
        {
            TextBlock->SetText(Text);
            return;
        }
    }

    // If no TextBlock found, the text might be set in Blueprint
    // In that case, we can't update it from C++, but the button will still work
    // You may need to expose a function or use a different approach in Blueprint
}

