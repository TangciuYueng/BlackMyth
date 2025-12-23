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
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMSaveLoadMenuWidget> SaveLoadMenuClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") bool bShowMainMenuOnStartup = true; // Ensure main menu auto-shows at startup

    // Ĭ����Ϸ��ͼ·�������� /Game/Maps/MyDefaultMap ���ʲ��� MyDefaultMap��
    UPROPERTY(EditDefaultsOnly, Category="Maps") FString DefaultGameMapPath;
};
