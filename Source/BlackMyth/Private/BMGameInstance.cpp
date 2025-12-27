// Fill out your copyright notice in the Description page of Project Settings.


#include "BMGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Character/Components/BMInventoryComponent.h"
#include "Character/Components/BMExperienceComponent.h"
#include "BlackMyth.h"
#include "Engine/World.h"
#include "UObject/Package.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "UObject/UObjectIterator.h"
#include "MoviePlayer.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Character/Components/BMStatsComponent.h"

void UBMGameInstance::CapturePlayerPersistentData(APlayerController* PC)
{
    bHasCapturedPersistentData = false;
    if (!PC) { return; }
    APawn* Pawn = PC->GetPawn();
    if (!Pawn) { return; }

    if (UBMInventoryComponent* Inv = Pawn->FindComponentByClass<UBMInventoryComponent>())
    {
        PersistentCoins = Inv->GetCurrency();
        PersistentItems = Inv->GetAllItems();
        PersistentItemCount = PersistentItems.Num();
    }

    if (UBMExperienceComponent* XP = Pawn->FindComponentByClass<UBMExperienceComponent>())
    {
        PersistentExp = static_cast<int32>(XP->GetCurrentXP());
        PersistentLevel = XP->GetLevel();
        PersistentSkillPoints = XP->GetSkillPoints();
        PersistentAttributePoints = XP->GetAttributePoints();
    }
    bHasCapturedPersistentData = true;
}

void UBMGameInstance::RestorePlayerPersistentData(APlayerController* PC)
{
    if (!bHasCapturedPersistentData || !PC) { return; }
    APawn* Pawn = PC->GetPawn();
    if (!Pawn) { return; }

    if (UBMInventoryComponent* Inv = Pawn->FindComponentByClass<UBMInventoryComponent>())
    {
        Inv->SetCurrencyDirect(PersistentCoins);
        // Clear and restore items
        // Direct access for restore; we can call ClearInventory() then add items
        Inv->ClearInventory();
        for (const auto& KVP : PersistentItems)
        {
            Inv->AddItem(KVP.Key, KVP.Value);
        }
        Inv->OnInventoryChanged.Broadcast();
    }
    if (UBMExperienceComponent* XP = Pawn->FindComponentByClass<UBMExperienceComponent>())
    {
        // 设置等级并应用成长数据（bApplyGrowth = true）
        XP->SetLevel(PersistentLevel, /*bApplyGrowth*/ true);
        XP->SetCurrentXP(PersistentExp, /*bCheckLevelUp*/ false);
        XP->SetSkillPoints(PersistentSkillPoints);
        XP->SetAttributePoints(PersistentAttributePoints);
    }
}

void UBMGameInstance::Init()
{
    Super::Init();
    // Optional: earlier implementation used a dedicated UBMNotifications subsystem.
    // Now notifications are handled via UBMUIManagerSubsystem + UBMEventBusSubsystem
    // and the DefaultNotificationWidgetClass can be set in editor via UBMGameInstance.NotificationClass
    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UBMGameInstance::HandlePostLoadMap);
    PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UBMGameInstance::HandlePreLoadMap);
}

void UBMGameInstance::Shutdown()
{
    if (PostLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
        PostLoadMapHandle.Reset();
    }
    if (PreLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
        PreLoadMapHandle.Reset();
    }
    Super::Shutdown();
}

void UBMGameInstance::HandlePostLoadMap(UWorld* LoadedWorld)
{
    if (!LoadedWorld)
    {
        return;
    }

    // Target maps and corresponding sound paths
    const FString TargetMapFullPath1 = TEXT("/Game/Stylized_PBR_Nature/Maps/Stylized_Nature_ExampleScene.Stylized_Nature_ExampleScene");
    const FString TargetMapPackage1 = TEXT("/Game/Stylized_PBR_Nature/Maps/Stylized_Nature_ExampleScene");
    const FString TargetMapBaseName1 = TEXT("Stylized_Nature_ExampleScene");

    const FString TargetMapFullPath2 = TEXT("/Game/Stylized_Spruce_Forest/Demo/Maps/STZD_Demo_01.STZD_Demo_01");
    const FString TargetMapPackage2 = TEXT("/Game/Stylized_Spruce_Forest/Demo/Maps/STZD_Demo_01");
    const FString TargetMapBaseName2 = TEXT("STZD_Demo_01");

    const FString WorldPathName = LoadedWorld->GetPathName();               // e.g. "World /Game/.../Map.Map"
    const FString PackageName = LoadedWorld->GetOutermost()->GetName();     // e.g. "/Game/.../Map"
    const FString Combined = PackageName + TEXT(".") + LoadedWorld->GetName(); // e.g. "/Game/.../Map.Map"
    const FString BaseLevelName = UGameplayStatics::GetCurrentLevelName(LoadedWorld, /*bRemovePrefixString=*/true); // Remove UEDPIE_ prefix

    const bool bIsTarget1 =
        WorldPathName.Contains(TargetMapFullPath1) ||
        Combined.Equals(TargetMapFullPath1, ESearchCase::IgnoreCase) ||
        PackageName.Equals(TargetMapPackage1, ESearchCase::IgnoreCase) ||
        BaseLevelName.Equals(TargetMapBaseName1, ESearchCase::IgnoreCase);

    const bool bIsTarget2 =
        WorldPathName.Contains(TargetMapFullPath2) ||
        Combined.Equals(TargetMapFullPath2, ESearchCase::IgnoreCase) ||
        PackageName.Equals(TargetMapPackage2, ESearchCase::IgnoreCase) ||
        BaseLevelName.Equals(TargetMapBaseName2, ESearchCase::IgnoreCase);

    const bool bIsTarget = bIsTarget1 || bIsTarget2;

    if (bIsTarget1 && !bHasPlayedIntroVideo)
    {
        UE_LOG(LogBlackMyth, Log, TEXT("HandlePostLoadMap: Fallback trigger for intro video on map '%s'."), *BaseLevelName);
        // If an intro video will play, postpone starting level music until video finishes
        PendingLevelMusicPath = bIsTarget2 ? TEXT("/Game/Audio/level2.level2") : TEXT("/Game/Audio/level1.level1");
        PlayIntroVideo();
        bHasPlayedIntroVideo = true;
    }

    if (!bIsTarget)
    {
        UE_LOG(LogBlackMyth, Verbose, TEXT("Map loaded but not target. WorldPath=%s, Package=%s, Combined=%s, BaseName=%s"), *WorldPathName, *PackageName, *Combined, *BaseLevelName);
        return;
    }

    const TCHAR* TargetSoundPath = bIsTarget2 ? TEXT("/Game/Audio/level2.level2") : TEXT("/Game/Audio/level1.level1");
    // If intro video is pending, defer playing until OnMoviePlaybackFinished
    if (bHasPlayedIntroVideo && !PendingLevelMusicPath.IsEmpty())
    {
        UE_LOG(LogBlackMyth, Log, TEXT("HandlePostLoadMap: Intro video pending, deferring level music: %s"), *PendingLevelMusicPath);
    }
    else
    {
        // On entering a new target level, ensure any preexisting audio is stopped.
        // For level2 specifically, stop all audio globally to avoid leftover level1 tracks from other worlds.
        const FString Level2Path = TEXT("/Game/Audio/level2.level2");
        if (FCString::Strcmp(TargetSoundPath, *Level2Path) == 0)
        {
            // Stop tracked component if present
            if (LevelMusicComp)
            {
                LevelMusicComp->Stop();
                LevelMusicComp = nullptr;
                CurrentLevelMusicPath.Empty();
            }
            // Stop all UAudioComponent instances regardless of world
            for (TObjectIterator<UAudioComponent> It; It; ++It)
            {
                UAudioComponent* AC = *It;
                if (!AC) continue;
                if (AC->IsPlaying())
                {
                    AC->Stop();
                }
            }
        }
        else
        {
            // Otherwise just stop audio in the target world
            ForceStopAllMusicInWorld(LoadedWorld);
        }

        StartLevelMusicForWorld(LoadedWorld, TargetSoundPath);
        UE_LOG(LogBlackMyth, Log, TEXT("Playing level music '%s' on map '%s' (BaseName=%s)"), TargetSoundPath, *Combined, *BaseLevelName);
    }
}

void UBMGameInstance::ForceStopAllMusicInWorld(UWorld* World)
{
    if (!World) return;

    // Stop tracked level music component if it belongs to this world
    if (LevelMusicComp && LevelMusicComp->GetWorld() == World)
    {
        LevelMusicComp->Stop();
        LevelMusicComp = nullptr;
        CurrentLevelMusicPath.Empty();
    }

    // Stop any other playing audio components in the world
    for (TObjectIterator<UAudioComponent> It; It; ++It)
    {
        UAudioComponent* AC = *It;
        if (!AC) continue;
        if (AC->GetWorld() != World) continue;
        if (AC->IsPlaying())
        {
            AC->Stop();
        }
    }
}

void UBMGameInstance::StopLevelMusic()
{
    if (LevelMusicComp)
    {
        // Protect boss phase 2 music: do not stop if boss phase 2 is active (not defeated) and the current music is boss2
        const FString Boss2MusicPath = TEXT("/Game/Audio/boss2.boss2");
        bool bPlayerDead = false;
        if (UWorld* W = GetWorld())
        {
            if (APlayerController* PC = W->GetFirstPlayerController())
            {
                if (APawn* Pawn = PC->GetPawn())
                {
                    if (UBMStatsComponent* Stats = Pawn->FindComponentByClass<UBMStatsComponent>())
                    {
                        bPlayerDead = Stats->IsDead();
                    }
                }
            }
        }

        if (bIsBossPhase2Defeated || bPlayerDead || CurrentLevelMusicPath.IsEmpty() || !CurrentLevelMusicPath.Equals(Boss2MusicPath, ESearchCase::IgnoreCase))
        {
            LevelMusicComp->Stop();
            LevelMusicComp = nullptr;
            CurrentLevelMusicPath.Empty();
        }
        else
        {
            UE_LOG(LogBlackMyth, Verbose, TEXT("StopLevelMusic: Skipping stop to preserve boss phase2 music: %s"), *CurrentLevelMusicPath);
        }
    }
}

void UBMGameInstance::StartLevelMusicForWorld(UWorld* World, const TCHAR* SoundPath)
{
    PlayMusic(World, SoundPath, /*bLoop=*/true);
}

void UBMGameInstance::OnLevelMusicFinished(UAudioComponent* AC)
{
    // Loop level music by restarting when finished
    if (AC && AC == LevelMusicComp && AC->Sound)
    {
        AC->Play(0.f);
    }
}

void UBMGameInstance::PlayMusic(UWorld* World, const TCHAR* SoundPath, bool bLoop)
{
    if (!World || !SoundPath)
    {
        return;
    }

    // Stop current music to ensure only one /Game/Audio track plays
    StopLevelMusic();

    // If playing death music, proactively stop any other audio components playing in this world
    // This ensures death music is the only audible track (stops ambient/other music tracks)
    const FString DeathMusicPath = TEXT("/Game/Audio/death.death");
    if (FCString::Strcmp(SoundPath, *DeathMusicPath) == 0)
    {
        for (TObjectIterator<UAudioComponent> It; It; ++It)
        {
            UAudioComponent* AC = *It;
            if (!AC) continue;
            // Only affect components that belong to the same world and are currently playing
            if (AC->IsPlaying() && AC->GetWorld() == World)
            {
                AC->Stop();
            }
        }
    }

    // General case: stop other audio components in this world to avoid overlapping level music
    // Respect boss2 protection: if boss phase2 music is active and should be preserved, skip stopping it
    const FString Boss2MusicPath = TEXT("/Game/Audio/boss2.boss2");
    bool bPlayerDead = false;
    if (UWorld* W = GetWorld())
    {
        if (APlayerController* PC = W->GetFirstPlayerController())
        {
            if (APawn* Pawn = PC->GetPawn())
            {
                if (UBMStatsComponent* Stats = Pawn->FindComponentByClass<UBMStatsComponent>())
                {
                    bPlayerDead = Stats->IsDead();
                }
            }
        }
    }

    const bool bPreserveBoss2 = (!bIsBossPhase2Defeated && !bPlayerDead && !CurrentLevelMusicPath.IsEmpty() && CurrentLevelMusicPath.Equals(Boss2MusicPath, ESearchCase::IgnoreCase));

    for (TObjectIterator<UAudioComponent> It; It; ++It)
    {
        UAudioComponent* AC = *It;
        if (!AC) continue;
        if (AC->GetWorld() != World) continue;
        if (!AC->IsPlaying()) continue;
        // Skip the current LevelMusicComp pointer if present (it will be handled by StopLevelMusic above)
        if (AC == LevelMusicComp) continue;
        // If we must preserve boss2 music, skip stopping components that are playing boss2
        if (bPreserveBoss2 && AC->Sound && AC->Sound->GetPathName().Contains(Boss2MusicPath))
        {
            continue;
        }
        AC->Stop();
    }

    if (USoundBase* SoundAsset = LoadObject<USoundBase>(nullptr, SoundPath))
    {
        // Record the current level music path for special-case logic (boss protection)
        CurrentLevelMusicPath = FString(SoundPath);

        LevelMusicComp = UGameplayStatics::SpawnSound2D(World, SoundAsset);
        if (LevelMusicComp)
        {
            if (bLoop)
            {
                LevelMusicComp->OnAudioFinishedNative.AddUObject(this, &UBMGameInstance::OnLevelMusicFinished);
            }
            // If level1 music started, trigger the intro/book UI immediately
            const FString Level1Path = TEXT("/Game/Audio/level1.level1");
            if (FCString::Strcmp(SoundPath, *Level1Path) == 0)
            {
                UE_LOG(LogBlackMyth, Log, TEXT("PlayMusic: Detected level1 music start, broadcasting OnIntroVideoFinishedNative to show book UI."));
                OnIntroVideoFinishedNative.Broadcast();
            }
        }
    }
    else
    {
        UE_LOG(LogBlackMyth, Warning, TEXT("Failed to load music asset: %s"), SoundPath);
    }
}

void UBMGameInstance::PlayIntroVideo()
{
    // Check if file exists
    const FString VideoPath = FPaths::ProjectContentDir() / TEXT("Movies/IntroVideo.mp4");
    if (!IFileManager::Get().FileExists(*VideoPath))
    {
        UE_LOG(LogBlackMyth, Error, TEXT("PlayIntroVideo: Video file not found at '%s'. Please ensure 'Content/Movies/IntroVideo.mp4' exists."), *VideoPath);
        return;
    }

    if (!GetMoviePlayer())
    {
        UE_LOG(LogBlackMyth, Warning, TEXT("PlayIntroVideo: MoviePlayer is NULL. Cannot play video."));
        return;
    }

    FLoadingScreenAttributes LoadingScreen;
    LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
    LoadingScreen.bMoviesAreSkippable = true;
    LoadingScreen.bWaitForManualStop = false;
    LoadingScreen.MoviePaths.Add(TEXT("IntroVideo")); // Assumes "Content/Movies/IntroVideo.mp4" exists
    LoadingScreen.PlaybackType = EMoviePlaybackType::MT_Normal;

    GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
    GetMoviePlayer()->PlayMovie();
    
    GetMoviePlayer()->OnMoviePlaybackFinished().AddUObject(this, &UBMGameInstance::OnMoviePlaybackFinished);
    UE_LOG(LogBlackMyth, Log, TEXT("PlayIntroVideo: Started playing IntroVideo."));
}

void UBMGameInstance::PlayEndVideo()
{
    // Check if file exists
    const FString VideoPath = FPaths::ProjectContentDir() / TEXT("Movies/EndVideo.mp4");
    if (!IFileManager::Get().FileExists(*VideoPath))
    {
        UE_LOG(LogBlackMyth, Error, TEXT("PlayEndVideo: Video file not found at '%s'. Please ensure 'Content/Movies/EndVideo.mp4' exists."), *VideoPath);
        return;
    }

    if (!GetMoviePlayer())
    {
        UE_LOG(LogBlackMyth, Warning, TEXT("PlayEndVideo: MoviePlayer is NULL. Cannot play video."));
        return;
    }

    // If configured to require a manual trigger, refuse to auto-play here.
    if (bRequireManualTriggerForEndVideo)
    {
        UE_LOG(LogBlackMyth, Log, TEXT("PlayEndVideo: Manual trigger required. Aborting automatic playback."));
        // Do not play automatically; caller should call RequestPlayEndVideo or PlayEndVideo when ready.
        return;
    }

    FLoadingScreenAttributes LoadingScreen;
    // Do not auto-complete when loading completes; allow movie to play its full duration
    LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
    // Allow movies to be skippable so player input (Enter) can dismiss the movie
    LoadingScreen.bMoviesAreSkippable = true;
    // Wait for manual stop: require explicit stop to end the movie playback
    LoadingScreen.bWaitForManualStop = true;
    LoadingScreen.MoviePaths.Add(TEXT("EndVideo")); // Assumes "Content/Movies/EndVideo.mp4" exists
    LoadingScreen.PlaybackType = EMoviePlaybackType::MT_Normal;

    GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
    GetMoviePlayer()->PlayMovie();

    // Mark playing flag
    bIsEndMoviePlaying = true;

    // Ensure we're not double-bound
    GetMoviePlayer()->OnMoviePlaybackFinished().RemoveAll(this);
    GetMoviePlayer()->OnMoviePlaybackFinished().AddUObject(this, &UBMGameInstance::OnEndVideoPlaybackFinished);
    UE_LOG(LogBlackMyth, Log, TEXT("PlayEndVideo: Started playing EndVideo (manual stop required)."));
}

void UBMGameInstance::RequestPlayEndVideo()
{
    // Allow manual play even if bRequireManualTriggerForEndVideo is true
    const bool bOld = bRequireManualTriggerForEndVideo;
    // Temporarily bypass the manual requirement to perform playback
    bRequireManualTriggerForEndVideo = false;
    PlayEndVideo();
    // Restore original behavior
    bRequireManualTriggerForEndVideo = bOld;
}

void UBMGameInstance::OnMoviePlaybackFinished()
{
    UE_LOG(LogBlackMyth, Log, TEXT("OnMoviePlaybackFinished: Movie finished."));
    GetMoviePlayer()->OnMoviePlaybackFinished().RemoveAll(this);
    // Notify any listeners (C++ subscribers)
    OnIntroVideoFinishedNative.Broadcast();

    // If there is pending level music, play it now
    if (!PendingLevelMusicPath.IsEmpty() && GetWorld())
    {
        StartLevelMusicForWorld(GetWorld(), *PendingLevelMusicPath);
        UE_LOG(LogBlackMyth, Log, TEXT("OnMoviePlaybackFinished: Starting pending level music '%s'"), *PendingLevelMusicPath);
        PendingLevelMusicPath.Empty();
    }
}

void UBMGameInstance::OnEndVideoPlaybackFinished()
{
    UE_LOG(LogBlackMyth, Log, TEXT("OnEndVideoPlaybackFinished: End Movie finished."));
    GetMoviePlayer()->OnMoviePlaybackFinished().RemoveAll(this);
    bHasWatchedEndVideo = true;
    bIsEndMoviePlaying = false;
    // End video finished can also notify (reuse same delegate if desired)
    OnIntroVideoFinishedNative.Broadcast();

    // If boss phase 2 was defeated, automatically return to main menu after end video
    if (bIsBossPhase2Defeated)
    {
        UE_LOG(LogBlackMyth, Log, TEXT("OnEndVideoPlaybackFinished: Boss Phase2 defeated - returning to main menu (emptymap)."));
        if (UWorld* W = GetWorld())
        {
            StopLevelMusic();
            UGameplayStatics::OpenLevel(W, TEXT("emptymap"));
        }
    }

    // Also play pending level music if any
    if (!PendingLevelMusicPath.IsEmpty() && GetWorld())
    {
        StartLevelMusicForWorld(GetWorld(), *PendingLevelMusicPath);
        UE_LOG(LogBlackMyth, Log, TEXT("OnEndVideoPlaybackFinished: Starting pending level music '%s'"), *PendingLevelMusicPath);
        PendingLevelMusicPath.Empty();
    }

}

void UBMGameInstance::StopEndVideo()
{
    if (!GetMoviePlayer())
    {
        UE_LOG(LogBlackMyth, Warning, TEXT("StopEndVideo: MoviePlayer is NULL."));
        return;
    }

    if (!bIsEndMoviePlaying)
    {
        UE_LOG(LogBlackMyth, Log, TEXT("StopEndVideo: No end movie is playing."));
        return;
    }

    // Movie player provides a manual stop method via ShutdownMoviePlayer
    if (auto* MP = GetMoviePlayer())
    {
        MP->Shutdown();
        UE_LOG(LogBlackMyth, Log, TEXT("StopEndVideo: MoviePlayer Shutdown called."));
    }

    bIsEndMoviePlaying = false;
    bHasWatchedEndVideo = true;
}


void UBMGameInstance::ResetBossMusicState()
{
    // Reset flags related to boss phase/end-video state and stop any level music
    bIsBossPhase2Defeated = false;
    bHasWatchedEndVideo = false;

    // Stop current level music if playing, but only if it's not boss2 music.
    // This avoids interrupting the boss2 track when boss enters phase2 recovery.
    const FString Boss2MusicPath = TEXT("/Game/Audio/boss2.boss2");
    if (!CurrentLevelMusicPath.Equals(Boss2MusicPath, ESearchCase::IgnoreCase))
    {
        StopLevelMusic();
    }
}

void UBMGameInstance::HandlePreLoadMap(const FString& MapName)
{
    // Ensure any playing music in the current world is stopped before a new map loads.
    if (UWorld* CurW = GetWorld())
    {
        ForceStopAllMusicInWorld(CurW);
    }
    // Check if we are loading the first level and haven't played the video yet
    // MapName usually comes as full path, e.g. "/Game/Stylized_PBR_Nature/Maps/Stylized_Nature_ExampleScene"
    
    const FString TargetMapPackage1 = TEXT("/Game/Stylized_PBR_Nature/Maps/Stylized_Nature_ExampleScene");
    
    UE_LOG(LogBlackMyth, Log, TEXT("HandlePreLoadMap: Loading map '%s'. Target is '%s'. bHasPlayedIntroVideo=%d"), *MapName, *TargetMapPackage1, bHasPlayedIntroVideo);

    if (!bHasPlayedIntroVideo && MapName.Contains(TargetMapPackage1))
    {
        UE_LOG(LogBlackMyth, Log, TEXT("HandlePreLoadMap: Detected target map load '%s', playing intro video."), *MapName);
        PlayIntroVideo();
        bHasPlayedIntroVideo = true;
    }
}

