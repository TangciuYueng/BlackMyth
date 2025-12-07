// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UButton;
#include "BMPauseMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMPauseMenuWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    UPROPERTY(meta=(BindWidget)) class UButton* ResumeButton = nullptr;
    UPROPERTY(meta=(BindWidget)) class UButton* SettingsButton = nullptr;
    UPROPERTY(meta=(BindWidget)) class UButton* QuitButton = nullptr;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    FDelegateHandle NotifyHandle;

    UFUNCTION()
    void OnResumeClicked();
    UFUNCTION()
    void OnSettingsClicked();
    UFUNCTION()
    void OnQuitClicked();
};
