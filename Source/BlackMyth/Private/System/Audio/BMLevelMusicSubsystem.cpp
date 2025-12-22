#include "System/Audio/BMLevelMusicSubsystem.h"

#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/UObjectGlobals.h"

const TCHAR* UBMLevelMusicSubsystem::Level1MapPath  = TEXT("/Game/Stylized_PBR_Nature/Maps/Stylized_Nature_ExampleScene");
const TCHAR* UBMLevelMusicSubsystem::Level2MapPath  = TEXT("/Game/Stylized_Spruce_Forest/Demo/Maps/STZD_Demo_01");

const TCHAR* UBMLevelMusicSubsystem::Level1MusicPath = TEXT("/Game/Audio/level1.level1");
const TCHAR* UBMLevelMusicSubsystem::Level2MusicPath = TEXT("/Game/Audio/level2.level2");
const TCHAR* UBMLevelMusicSubsystem::DeathMusicPath  = TEXT("/Game/Audio/death.death");

void UBMLevelMusicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMapWithWorld);

    if (UWorld* World = GetWorld())
    {
        PlayLevelMusicForWorld(World);
    }
}

void UBMLevelMusicSubsystem::Deinitialize()
{
    if (PostLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
        PostLoadMapHandle.Reset();
    }

    StopCurrentTrack();
    MusicComponent = nullptr;
    CurrentSound = nullptr;

    Super::Deinitialize();
}

void UBMLevelMusicSubsystem::EnsureAudioComponent(UWorld* World)
{
    if (MusicComponent && !MusicComponent->IsBeingDestroyed())
    {
        return;
    }

    if (!World)
    {
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    UObject* Outer = PC ? static_cast<UObject*>(PC) : static_cast<UObject*>(World);

    UAudioComponent* NewComp = NewObject<UAudioComponent>(Outer);
    if (!NewComp)
    {
        return;
    }

    NewComp->bAutoActivate = false;
    NewComp->bIsUISound = true;
    NewComp->RegisterComponentWithWorld(World);

    MusicComponent = NewComp;
}

void UBMLevelMusicSubsystem::StopCurrentTrack()
{
    if (MusicComponent)
    {
        MusicComponent->Stop();
        MusicComponent->SetSound(nullptr);
    }
    CurrentSound = nullptr;
}

void UBMLevelMusicSubsystem::PlayTrackByPath(const FString& SoundAssetPath)
{
    if (SoundAssetPath.IsEmpty())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    EnsureAudioComponent(World);
    if (!MusicComponent)
    {
        return;
    }

    USoundBase* Sound = LoadObject<USoundBase>(nullptr, *SoundAssetPath);
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("BMLevelMusicSubsystem: Failed to load sound: %s"), *SoundAssetPath);
        return;
    }

    if (CurrentSound == Sound && MusicComponent->IsPlaying())
    {
        return;
    }

    MusicComponent->Stop();
    MusicComponent->SetSound(Sound);
    MusicComponent->Play(0.f);

    CurrentSound = Sound;
}

void UBMLevelMusicSubsystem::PlayLevelMusicForWorld(UWorld* World)
{
    if (!World)
    {
        return;
    }

    const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World, true);

    if (CurrentLevelName.IsEmpty())
    {
        return;
    }

    if (CurrentLevelName == TEXT("Stylized_Nature_ExampleScene"))
    {
        PlayTrackByPath(Level1MusicPath);
        return;
    }

    if (CurrentLevelName == TEXT("STZD_Demo_01"))
    {
        PlayTrackByPath(Level2MusicPath);
        return;
    }
}

void UBMLevelMusicSubsystem::PlayDeathMusic()
{
    PlayTrackByPath(DeathMusicPath);
}

void UBMLevelMusicSubsystem::HandlePostLoadMapWithWorld(UWorld* LoadedWorld)
{
    PlayLevelMusicForWorld(LoadedWorld);
}
