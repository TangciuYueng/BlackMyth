// Fill out your copyright notice in the Description page of Project Settings.


#include "BMGameInstance.h"
#include "BlackMyth.h"

#include "Engine/World.h"
#include "UObject/Package.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

void UBMGameInstance::Init()
{
    Super::Init();
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
    if (!World || !SoundPath)
    {
        return;
    }

    // Stop any previous
    StopLevelMusic();

    if (USoundBase* SoundAsset = LoadObject<USoundBase>(nullptr, SoundPath))
    {
        LevelMusicComp = UGameplayStatics::SpawnSound2D(World, SoundAsset);
    }
    else
    {
        UE_LOG(LogBlackMyth, Warning, TEXT("Failed to load level music asset: %s"), SoundPath);
    }
}

