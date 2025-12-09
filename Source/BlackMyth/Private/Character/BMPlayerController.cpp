// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BMPlayerController.h"
#include "BMGameInstance.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "UI/BMMainWidget.h"
#include "UI/BMPauseMenuWidget.h"
#include "InputCoreTypes.h"

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
    if (InputComponent)
    {
        // 仅使用 C++ 绑定，不依赖 Project Settings 的 Action Mappings
        // 直接绑定 Tab 键以打开暂停菜单
        InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ABMPlayerController::TogglePauseMenu);
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Bound Tab to TogglePauseMenu via C++"));
    }
}

void ABMPlayerController::TogglePauseMenu()
{
    UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: TogglePauseMenu invoked"));
    if (UGameInstance* GI = GetGameInstance())
    {
        UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>();
        UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
        if (UIManager)
        {
            if (UIManager->IsPauseMenuVisible())
            {
                UIManager->HidePauseMenu();
                FInputModeGameOnly GameOnly;
                SetInputMode(GameOnly);
                bShowMouseCursor = false;
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Hide Pause -> GameOnly"));
                return;
            }

            TSubclassOf<UBMPauseMenuWidget> PauseClass = nullptr;
            if (BMGI && BMGI->PauseMenuClass.IsValid())
            {
                PauseClass = BMGI->PauseMenuClass.Get();
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Using GameInstance PauseMenuClass."));
            }
            if (!PauseClass)
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Trying /Game/UI/WBP_PauseMenu.WBP_PauseMenu_C"));
                PauseClass = LoadClass<UBMPauseMenuWidget>(nullptr, TEXT("/Game/UI/WBP_PauseMenu.WBP_PauseMenu_C"));
            }
            if (!PauseClass)
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Trying /Game/BlackMyth/UI/WBP_PauseMenu.WBP_PauseMenu_C"));
                PauseClass = LoadClass<UBMPauseMenuWidget>(nullptr, TEXT("/Game/BlackMyth/UI/WBP_PauseMenu.WBP_PauseMenu_C"));
            }

            if (PauseClass)
            {
                UIManager->ShowPauseMenu(PauseClass);
                // 切换为 UI Only 模式以便暂停菜单可交互
                FInputModeUIOnly InputMode;
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                SetInputMode(InputMode);
                bShowMouseCursor = true;
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: TogglePauseMenu -> UIOnly + cursor"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("ABMPlayerController: PauseMenu class not found. Check GI setting or asset path."));
            }
        }
    }
}

