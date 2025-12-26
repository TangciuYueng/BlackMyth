// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BMGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    // ���ã�UI��ͼ��·������
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMMainWidget> MainMenuClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMPauseMenuWidget> PauseMenuClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMHUDWidget> HUDClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMNotificationWidget> NotificationClass;
    // Default notification widget blueprint (optional). If set, will be pushed to UBMNotifications on startup.
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UUserWidget> DefaultNotificationWidgetClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMSaveLoadMenuWidget> SaveLoadMenuClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") bool bShowMainMenuOnStartup = true; // Ensure main menu auto-shows at startup

    // Ĭ����Ϸ��ͼ·�������� /Game/Maps/MyDefaultMap ���ʲ��� MyDefaultMap��
    UPROPERTY(EditDefaultsOnly, Category="Maps") FString DefaultGameMapPath;

public:
    // Native multicast for intro video finished (C++ subscribers)
    FSimpleMulticastDelegate OnIntroVideoFinishedNative;

    // Simple persistent data kept across level reloads
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentCoins = 0;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentExp = 0;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentItemCount = 0; // optional aggregate
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentLevel = 1;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentSkillPoints = 0;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") int32 PersistentAttributePoints = 0;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") TMap<FName, int32> PersistentItems;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") bool bHasCapturedPersistentData = false;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") bool bHasPlayedIntroVideo = false;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") bool bIsBossPhase2Defeated = false;
    UPROPERTY(BlueprintReadWrite, Category="Persistent") bool bHasWatchedEndVideo = false;

    // Capture from player state/components before reload
    UFUNCTION(BlueprintCallable, Category="Persistent") void CapturePlayerPersistentData(APlayerController* PC);
    // Restore to player state/components after reload
    UFUNCTION(BlueprintCallable, Category="Persistent") void RestorePlayerPersistentData(APlayerController* PC);
protected:
    virtual void Init() override;
    virtual void Shutdown() override;

private:
    FDelegateHandle PostLoadMapHandle;
    void HandlePostLoadMap(class UWorld* LoadedWorld);

public:
    void StopLevelMusic();
    void StartLevelMusicForWorld(class UWorld* World, const TCHAR* SoundPath);
    void OnLevelMusicFinished(class UAudioComponent* AC);
    void PlayMusic(class UWorld* World, const TCHAR* SoundPath, bool bLoop);
    void PlayIntroVideo();
    void OnMoviePlaybackFinished();
    void PlayEndVideo();
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
    FDelegateHandle PreLoadMapHandle;
    void HandlePreLoadMap(const FString& MapName);

    // If intro video plays, delay level music until video finishes
    FString PendingLevelMusicPath;

    // Current playing level music asset path (used to protect boss2 music from being stopped)
    FString CurrentLevelMusicPath;

    UPROPERTY(Transient)
    class UAudioComponent* LevelMusicComp = nullptr;
};
