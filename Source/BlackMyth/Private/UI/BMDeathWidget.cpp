#include "UI/BMDeathWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "BMGameInstance.h"
#include "UI/BMMainWidget.h"
#include "Character/Components/BMStatsComponent.h"

/*
 * @brief Native construct, it native construct
 */
void UBMDeathWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UBMDeathWidget::OnRestartClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UBMDeathWidget::OnQuitClicked);
	}
}

/*
 * @brief Native destruct, it native destruct
 */
void UBMDeathWidget::NativeDestruct()
{
	if (RestartButton)
	{
		RestartButton->OnClicked.RemoveAll(this);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.RemoveAll(this);
	}
	Super::NativeDestruct();
}

/*
 * @brief On restart clicked, it on restart clicked
 */
void UBMDeathWidget::OnRestartClicked()
{
    // Reload current level (clean reset of animations, movement, input, etc.)
    if (UWorld* World = GetWorld())
    {
        const FName MapName = FName(*UGameplayStatics::GetCurrentLevelName(this, /*bRemovePrefixPIE*/ true));
        // Capture persistent data (coins/exp/items) in GI before reload
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (auto* BMGI = Cast<UBMGameInstance>(GI))
            {
                if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
                {
                    BMGI->CapturePlayerPersistentData(PC);
                }
            }
        }
        UGameplayStatics::OpenLevel(this, MapName);
    }
}

/*
 * @brief On quit clicked, it on quit clicked
 */
void UBMDeathWidget::OnQuitClicked()
{
    // Show main menu instead of quitting the application
    if (UWorld* World = GetWorld())
    {
        UGameInstance* GI = World->GetGameInstance();
        UBMUIManagerSubsystem* UI = GI ? GI->GetSubsystem<UBMUIManagerSubsystem>() : nullptr;
        const UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
        if (UI)
        {
            // Hide the death widget first
            UI->HideDeath();

            // Resolve main menu class
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

                // Switch to UI-only input and show cursor for main menu interaction
                if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
                {
                    FInputModeUIOnly InputMode;
                    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                    PC->SetInputMode(InputMode);
                    PC->bShowMouseCursor = true;
                }
            }
        }
    }
}
