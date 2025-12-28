// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UButton;
class UTextBlock;
#include "BMPauseMenuWidget.generated.h"

/**
 * @brief Define the UBMPauseMenuWidget class
 * @param UBMPauseMenuWidget The name of the class
 * @param UBMWidgetBase The parent class
 */
UCLASS()
class BLACKMYTH_API UBMPauseMenuWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    // Resume button binding
    UPROPERTY(meta=(BindWidget)) class UButton* ResumeButton = nullptr;
    // Save button binding
    UPROPERTY(meta=(BindWidget)) class UButton* SaveButton = nullptr;
    // Return to main button binding
    UPROPERTY(meta=(BindWidget)) class UButton* ReturnToMainButton = nullptr;


protected:
    // Native construct
    virtual void NativeConstruct() override;
    // Native destruct
    virtual void NativeDestruct() override;
    // Bind event bus
    virtual void BindEventBus(class UBMEventBusSubsystem* EventBus) override;
    // Unbind event bus
    virtual void UnbindEventBus(class UBMEventBusSubsystem* EventBus) override;

private:
    // Cached bindings
    FDelegateHandle HealthChangedHandle;

    // On resume clicked
    UFUNCTION()
    void OnResumeClicked();
    // On save clicked
    UFUNCTION()
    void OnSaveClicked();
    // On return to main clicked
    UFUNCTION()
    void OnReturnToMainClicked();
    // On skill tree clicked
    UFUNCTION()
    void OnSkillTreeClicked();
    // On equipment upgrade clicked
    UFUNCTION()
    void OnEquipmentUpgradeClicked();
    // On settings clicked
    UFUNCTION()
    void OnSettingsClicked();
};
