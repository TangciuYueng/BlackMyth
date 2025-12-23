// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMMainWidget.h"
#include "Components/Button.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "UI/BMSaveLoadMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "BMGameInstance.h"

void UBMMainWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (StartButton)
    {
        StartButton->OnClicked.AddDynamic(this, &UBMMainWidget::OnStartClicked);
    }
    if (BossBattle)
    {
        BossBattle->OnClicked.AddDynamic(this, &UBMMainWidget::OnBossBattleClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UBMMainWidget::OnQuitClicked);
    }
    if (SaveLoadButton)
    {
        SaveLoadButton->OnClicked.AddDynamic(this, &UBMMainWidget::OnSaveLoadClicked);
    }
}

void UBMMainWidget::NativeDestruct()
{
    if (StartButton)
    {
        StartButton->OnClicked.RemoveAll(this);
    }
    if (BossBattle)
    {
        BossBattle->OnClicked.RemoveAll(this);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.RemoveAll(this);
    }
    if (SaveLoadButton)
    {
        SaveLoadButton->OnClicked.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UBMMainWidget::OnStartClicked()
{
    // �л�����Ϸ����ģʽ�����ز˵�
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
        }
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>())
            {
                // ȷ�����˵�������
                UIManager->HideAllMenus();
            }
        }


        UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Stylized_PBR_Nature/Maps/Stylized_Nature_ExampleScene")));

    }
}

void UBMMainWidget::OnQuitClicked()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            UKismetSystemLibrary::QuitGame(World, PC, EQuitPreference::Quit, false);
        }
    }
}

void UBMMainWidget::OnBossBattleClicked()
{
    // �е���Ϸ���롢���ز˵����л��� Boss ս��ͼ
    if (UWorld* World = GetWorld())
    {

        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
        }
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>())
            {
                // ȷ�����˵�������
                UIManager->HideAllMenus();
            }
        }

        // �л�����һ����ͼ��Stylized_Spruce_Forest��λ�� Content/Stylized_PBR_Nature �£�
        UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Stylized_Spruce_Forest/Demo/Maps/STZD_Demo_01")));

    }
}

void UBMMainWidget::OnSaveLoadClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(FText::FromString(TEXT("Open Save/Load Menu")));
    }
    // Hide main menu and show SaveLoad menu
    UWorld* World = GetWorld();
    if (!World) return;
    if (UBMUIManagerSubsystem* UI = GetUIManager())
    {
        UI->HideMainMenu();

        // Resolve SaveLoad menu class from GameInstance or fallback paths
        UBMGameInstance* BMGI = Cast<UBMGameInstance>(World->GetGameInstance());
        TSubclassOf<UBMSaveLoadMenuWidget> SaveLoadClass = nullptr;
        if (BMGI && BMGI->SaveLoadMenuClass.IsValid())
        {
            SaveLoadClass = BMGI->SaveLoadMenuClass.Get();
        }
        if (!SaveLoadClass)
        {
            SaveLoadClass = LoadClass<UBMSaveLoadMenuWidget>(nullptr, TEXT("/Game/UI/WBP_LoadMenu"));
        }
        if (!SaveLoadClass)
        {
            SaveLoadClass = LoadClass<UBMSaveLoadMenuWidget>(nullptr, TEXT("/Game//UI/WBP_LoadMenu.WBP_LoadMenu_C"));
        }
        if (SaveLoadClass)
        {
            UI->ShowSaveLoadMenu(SaveLoadClass);
        }
    }

    // Ensure UI input mode for SaveLoad menu interaction
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
}