// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BMPlayerController.h"
#include "BMGameInstance.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "UI/BMMainWidget.h"
#include "UI/BMPauseMenuWidget.h"

void ABMPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UGameInstance* GI = GetGameInstance())
    {
        UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>();
        UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
        if (UIManager)
        {
            TSubclassOf<UBMMainWidget> ClassToUse = nullptr;
            if (BMGI && BMGI->MainMenuClass.IsValid())
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Using GameInstance MainMenuClass."));
                ClassToUse = BMGI->MainMenuClass.Get();
            }
            if (!ClassToUse)
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Trying /Game/UI/WBP_MainMenu.WBP_MainMenu_C"));
                ClassToUse = LoadClass<UBMMainWidget>(nullptr, TEXT("/Game/UI/WBP_MainMenu.WBP_MainMenu_C"));
            }
            if (!ClassToUse)
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Trying /Game/BlackMyth/UI/WBP_MainMenu.WBP_MainMenu_C"));
                ClassToUse = LoadClass<UBMMainWidget>(nullptr, TEXT("/Game/BlackMyth/UI/WBP_MainMenu.WBP_MainMenu_C"));
            }

            if (ClassToUse)
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Showing MainMenu with class %s"), *ClassToUse->GetName());
                UIManager->ShowMainMenu(ClassToUse);

                // 切换为 UI Only 模式，显示鼠标以确保菜单可见和可交互
                FInputModeUIOnly InputMode;
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                SetInputMode(InputMode);
                bShowMouseCursor = true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("ABMPlayerController: MainMenu class not found. Check path or GameInstance config."));
            }
        }
    }
}

void ABMPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    // 绑定示例按键：自定义 Action "TogglePause" 在项目设置里创建
    if (InputComponent)
    {
        InputComponent->BindAction("TogglePause", IE_Pressed, this, &ABMPlayerController::TogglePauseMenu);
    }
}

void ABMPlayerController::TogglePauseMenu()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>();
        UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
        if (UIManager && BMGI && BMGI->PauseMenuClass.IsValid())
        {
            UIManager->ShowPauseMenu(BMGI->PauseMenuClass.Get());
        }
    }
}

