// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UButton;
class UTextBlock;
#include "BMPauseMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMPauseMenuWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    // Buttons
    UPROPERTY(meta=(BindWidget)) class UButton* ResumeButton = nullptr;
    UPROPERTY(meta=(BindWidget)) class UButton* SaveButton = nullptr;
    UPROPERTY(meta=(BindWidget)) class UButton* ReturnToMainButton = nullptr;


protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void BindEventBus(class UBMEventBusSubsystem* EventBus) override;
    virtual void UnbindEventBus(class UBMEventBusSubsystem* EventBus) override;

private:
    // Cached bindings
    FDelegateHandle HealthChangedHandle;

    UFUNCTION()
    void OnResumeClicked();
    UFUNCTION()
    void OnSaveClicked();
    UFUNCTION()
    void OnSkillTreeClicked();
    UFUNCTION()
    void OnEquipmentUpgradeClicked();
    UFUNCTION()
    void OnSettingsClicked();
    UFUNCTION()
    void OnReturnToMainClicked();
};
