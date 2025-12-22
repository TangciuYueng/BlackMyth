// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMMainWidget.h"
#include "Components/Button.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"

void UBMMainWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (StartButton)
    {
        StartButton->OnClicked.AddDynamic(this, &UBMMainWidget::OnStartClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UBMMainWidget::OnQuitClicked);
    }
}

void UBMMainWidget::NativeDestruct()
{
    if (StartButton)
    {
        StartButton->OnClicked.RemoveAll(this);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UBMMainWidget::OnStartClicked()
{
    // 切换到游戏输入模式并隐藏菜单
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
                UIManager->HideAllMenus();
            }
        }
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

