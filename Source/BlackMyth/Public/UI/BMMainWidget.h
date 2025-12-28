// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UButton;
#include "BMMainWidget.generated.h"

/**
 * @brief Define the UBMMainWidget class
 * @param UBMMainWidget The name of the class
 * @param UBMWidgetBase The parent class
 */
UCLASS()
class BLACKMYTH_API UBMMainWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    // Start button binding
    UPROPERTY(meta=(BindWidget)) class UButton* StartButton = nullptr;
    // Boss battle button binding
    UPROPERTY(meta=(BindWidget)) class UButton* BossBattle = nullptr;
    // Quit button binding
    UPROPERTY(meta=(BindWidget)) class UButton* QuitButton = nullptr;
    // Save load button binding
    UPROPERTY(meta=(BindWidget)) class UButton* SaveLoadButton = nullptr;

protected:
    // Native construct
    virtual void NativeConstruct() override;
    // Native destruct
    virtual void NativeDestruct() override;

private:
    // On start clicked
    UFUNCTION()
    void OnStartClicked();
    // On boss battle clicked
    UFUNCTION()
    void OnBossBattleClicked();
    // On quit clicked
    UFUNCTION()
    void OnQuitClicked();
    // On save load clicked
    UFUNCTION()
    void OnSaveLoadClicked();
};
