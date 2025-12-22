#include "BMGameModeBase.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/BMPlayerController.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "UI/BMHUDWidget.h"
#include "UI/BMNotificationWidget.h"
#include "BMGameInstance.h"
#include "Character/BMPlayerController.h"

#include "Sound/SoundBase.h"
#include "System/Audio/BMLevelMusicSubsystem.h"

ABMGameModeBase::ABMGameModeBase()
{
    DefaultPawnClass = ABMPlayerCharacter::StaticClass();
    PlayerControllerClass = ABMPlayerController::StaticClass();
<<<<<<< Updated upstream
=======
    HUDClass = ABMGameHUD::StaticClass();

	static ConstructorHelpers::FObjectFinder<USoundBase> Level1Finder(
		TEXT("/Script/Engine.SoundWave'/Game/Audio/level1.level1'")
	);
	if (Level1Finder.Succeeded())
	{
		Level1StartMusic = Level1Finder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> Level2Finder(
		TEXT("/Script/Engine.SoundWave'/Game/Audio/level2.level2'")
	);
	if (Level2Finder.Succeeded())
	{
		Level2StartMusic = Level2Finder.Object;
	}
>>>>>>> Stashed changes
}

void ABMGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    UGameInstance* GI = GetGameInstance();
<<<<<<< Updated upstream
    if (!GI) return;
=======
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("BMGameModeBase::BeginPlay: GameInstance is null"));
        return;
    }

    // Diagnostics: log current map and GI class
    const UWorld* World = GetWorld();
    const FString CurrentLevelName = World ? UGameplayStatics::GetCurrentLevelName(const_cast<UObject*>(static_cast<const UObject*>(World)), true) : TEXT("<no world>");
	const FString CurrentWorldPath = World ? World->GetPathName() : TEXT("<no world>");
    UE_LOG(LogTemp, Log, TEXT("BMGameModeBase::BeginPlay on map '%s' (WorldPath=%s), GI class: %s"), *CurrentLevelName, *CurrentWorldPath, *GI->GetClass()->GetName());

	// Level start music (exact map asset paths)
	USoundBase* MusicToPlay = nullptr;
	if (CurrentWorldPath == TEXT("/Game/Stylized_PBR_Nature/Maps/Stylized_Nature_ExampleScene.Stylized_Nature_ExampleScene"))
	{
		MusicToPlay = Level1StartMusic;
	}
	else if (CurrentWorldPath == TEXT("/Game/Stylized_Spruce_Forest/Demo/Maps/STZD_Demo_01.STZD_Demo_01"))
	{
		MusicToPlay = Level2StartMusic;
	}

	if (UBMLevelMusicSubsystem* Music = GI->GetSubsystem<UBMLevelMusicSubsystem>())
	{
		Music->PlayLevelMusicForWorld(GetWorld());
	}
>>>>>>> Stashed changes

    UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>();
    UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
    if (!UIManager || !BMGI) return;

    if (BMGI->HUDClass.IsValid())
    {
        UIManager->ShowHUD(BMGI->HUDClass.Get());
    }
    if (BMGI->NotificationClass.IsValid())
    {
        UIManager->ShowNotification(BMGI->NotificationClass.Get());
    }
}


