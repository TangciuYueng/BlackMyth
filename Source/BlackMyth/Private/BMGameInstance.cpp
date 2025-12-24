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
        XP->SetLevel(PersistentLevel, /*bApplyGrowth*/ false);
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
}

void UBMGameInstance::Shutdown()
{
    if (PostLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
        PostLoadMapHandle.Reset();
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

    if (!bIsTarget)
    {
        UE_LOG(LogBlackMyth, Verbose, TEXT("Map loaded but not target. WorldPath=%s, Package=%s, Combined=%s, BaseName=%s"), *WorldPathName, *PackageName, *Combined, *BaseLevelName);
        return;
    }

    const TCHAR* TargetSoundPath = bIsTarget2 ? TEXT("/Game/Audio/level2.level2") : TEXT("/Game/Audio/level1.level1");
    StartLevelMusicForWorld(LoadedWorld, TargetSoundPath);
    UE_LOG(LogBlackMyth, Log, TEXT("Playing level music '%s' on map '%s' (BaseName=%s)"), TargetSoundPath, *Combined, *BaseLevelName);
}

void UBMGameInstance::StopLevelMusic()
{
    if (LevelMusicComp)
    {
        LevelMusicComp->Stop();
        LevelMusicComp = nullptr;
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

    if (USoundBase* SoundAsset = LoadObject<USoundBase>(nullptr, SoundPath))
    {
        LevelMusicComp = UGameplayStatics::SpawnSound2D(World, SoundAsset);
        if (LevelMusicComp)
        {
            if (bLoop)
            {
                LevelMusicComp->OnAudioFinishedNative.AddUObject(this, &UBMGameInstance::OnLevelMusicFinished);
            }
        }
    }
    else
    {
        UE_LOG(LogBlackMyth, Warning, TEXT("Failed to load music asset: %s"), SoundPath);
    }
}

