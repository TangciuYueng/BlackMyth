// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMMainWidget.h"
#include "Components/Button.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
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
                // 确保主菜单被隐藏
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
    // 切到游戏输入、隐藏菜单并切换到 Boss 战地图
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
                // 确保主菜单被隐藏
                UIManager->HideAllMenus();
            }
        }

        // 切换到另一个地图：Stylized_Spruce_Forest（位于 Content/Stylized_PBR_Nature 下）
        UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Stylized_Spruce_Forest/Demo/Maps/STZD_Demo_01")));

    }
}

