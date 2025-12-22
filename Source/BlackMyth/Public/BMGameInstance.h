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
    // ≈‰÷√£∫UI¿∂Õº¿‡¬∑æ∂ª∫¥Ê
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMMainWidget> MainMenuClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMPauseMenuWidget> PauseMenuClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMHUDWidget> HUDClass;
    UPROPERTY(EditDefaultsOnly, Category="UI") TSoftClassPtr<class UBMNotificationWidget> NotificationClass;
};
