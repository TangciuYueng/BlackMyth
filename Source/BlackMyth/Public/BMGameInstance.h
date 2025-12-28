// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BMGameInstance.generated.h"

/**
 * @brief Define the UBMGameInstance class
 * @param UBMGameInstance The name of the class
 * @param UGameInstance The parent class
 */
UCLASS()
class BLACKMYTH_API UBMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    // Main menu class
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMMainWidget> MainMenuClass;
    // Pause menu class
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMPauseMenuWidget> PauseMenuClass;
    // HUD class
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMHUDWidget> HUDClass;
    // Notification class
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMNotificationWidget> NotificationClass;
    // Default notification widget blueprint (optional). If set, will be pushed to UBMNotifications on startup.
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UUserWidget> DefaultNotificationWidgetClass;
    // Save load menu class
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMSaveLoadMenuWidget> SaveLoadMenuClass;
    // Show main menu on startup
    UPROPERTY(EditDefaultsOnly, Category="UI") bool bShowMainMenuOnStartup = true; // Ensure main menu auto-shows at startup

    // Default game map path
    UPROPERTY(EditDefaultsOnly, Category="Maps") FString DefaultGameMapPath;

public:
    // Native multicast for intro video finished (C++ subscribers)
    FSimpleMulticastDelegate OnIntroVideoFinishedNative;

    // Simple persistent data kept across level reloads
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentCoins = 0;
    // Persistent experience
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentExp = 0;
    // Persistent item count
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentItemCount = 0; // optional aggregate
    // Persistent level
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentLevel = 1;
    // Persistent skill points
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentSkillPoints = 0;
    // Persistent attribute points
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentAttributePoints = 0;
    // Persistent items
    UPROPERTY(BlueprintReadWrite, Category="Persistent") TMap<FName, int32> PersistentItems;
    // Has captured persistent data
    UPROPERTY(BlueprintReadWrite, Category="Persistent") bool bHasCapturedPersistentData = false;
    // Has played intro video
    UPROPERTY(BlueprintReadWrite, Category="Persistent") bool bHasPlayedIntroVideo = false;

    /** (removed) */
    UPROPERTY(BlueprintReadWrite, Category="Persistent") bool bIsBossPhase2Defeated = false;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") bool bHasWatchedEndVideo = false;

    // Capture from player state/components before reload
    UFUNCTION(BlueprintCallable, Category="Persistent") void CapturePlayerPersistentData(APlayerController* PC);
    // Restore to player state/components after reload
    UFUNCTION(BlueprintCallable, Category="Persistent") void RestorePlayerPersistentData(APlayerController* PC);
protected:
    // Init the game instance
    virtual void Init() override;
    // Shutdown the game instance
    virtual void Shutdown() override;

private:
    // Post load map handle
    FDelegateHandle PostLoadMapHandle;
    // Handle post load map
    void HandlePostLoadMap(class UWorld* LoadedWorld);

public:
    // Stop level music
    void StopLevelMusic();
    // Start level music for world
    void StartLevelMusicForWorld(class UWorld* World, const TCHAR* SoundPath);
    // On level music finished
    void OnLevelMusicFinished(class UAudioComponent* AC);
    // Play music
    void PlayMusic(class UWorld* World, const TCHAR* SoundPath, bool bLoop);
    // Play intro video
    void PlayIntroVideo();
    // On movie playback finished
    void OnMoviePlaybackFinished();
    // Play end video
    void PlayEndVideo();
    // On end video playback finished
    void OnEndVideoPlaybackFinished();
    // If true, EndVideo will only play after a manual request (e.g. player presses Enter)
    UPROPERTY(EditDefaultsOnly, Category="Video")
    bool bRequireManualTriggerForEndVideo = true;

    // Request the EndVideo to play (marks an internal flag that PlayEndVideo will consume)
    UFUNCTION(BlueprintCallable, Category="Video")
    void RequestPlayEndVideo();

    /** True while the EndVideo movie is currently playing */
    UPROPERTY(BlueprintReadOnly, Transient, Category="Video")
    bool bIsEndMoviePlaying = false;

    /** Stop the EndVideo immediately (used for manual stop) */
    UFUNCTION(BlueprintCallable, Category="Video")
    void StopEndVideo();
    // Reset boss-related music flags and stop level music (used when boss is revived)
    UFUNCTION(BlueprintCallable, Category="Audio")
    void ResetBossMusicState();

private:
    // Pre load map handle
    FDelegateHandle PreLoadMapHandle;
    // Handle pre load map
    void HandlePreLoadMap(const FString& MapName);

    // Force-stop any audio components playing in the specified world and clear level music state
    void ForceStopAllMusicInWorld(UWorld* World);

    // If intro video plays, delay level music until video finishes
    FString PendingLevelMusicPath;

    // Current playing level music asset path (used to protect boss2 music from being stopped)
    FString CurrentLevelMusicPath;

    UPROPERTY(Transient)
    // Level music component
    class UAudioComponent* LevelMusicComp = nullptr;
};
