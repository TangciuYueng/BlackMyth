#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BMLevelMusicSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS()
class BLACKMYTH_API UBMLevelMusicSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "BM|Audio")
    void PlayTrackByPath(const FString& SoundAssetPath);

    UFUNCTION(BlueprintCallable, Category = "BM|Audio")
    void StopCurrentTrack();

    UFUNCTION(BlueprintCallable, Category = "BM|Audio")
    void PlayLevelMusicForWorld(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "BM|Audio")
    void PlayDeathMusic();

private:
    void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);
    void EnsureAudioComponent(UWorld* World);

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> MusicComponent = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<USoundBase> CurrentSound = nullptr;

    FDelegateHandle PostLoadMapHandle;

private:
    static const TCHAR* Level1MapPath;
    static const TCHAR* Level2MapPath;

    static const TCHAR* Level1MusicPath;
    static const TCHAR* Level2MusicPath;
    static const TCHAR* DeathMusicPath;
};
