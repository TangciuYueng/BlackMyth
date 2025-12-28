// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UProgressBar;
class UTextBlock;
#include "BMBossBarBase.generated.h"

/**
 * @brief Define the UBMBossBarBase class
 * @param UBMBossBarBase The name of the class
 * @param UBMWidgetBase The parent class
 */
UCLASS()
class BLACKMYTH_API UBMBossBarBase : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    // Boss health bar binding
    UPROPERTY(meta=(BindWidget)) class UProgressBar* BossHealthBar = nullptr;
    // Phase hint text binding
    UPROPERTY(meta=(BindWidget)) class UTextBlock* PhaseHintText = nullptr;

    // Native construct
    virtual void NativeConstruct() override;

protected:
    // Bind event bus
    virtual void BindEventBus(class UBMEventBusSubsystem* EventBus) override;
    // Unbind event bus
    virtual void UnbindEventBus(class UBMEventBusSubsystem* EventBus) override;

private:
    // Boss health handle
    FDelegateHandle BossHealthHandle;
    // Boss phase handle
    FDelegateHandle BossPhaseHandle;

    // Handle boss health
    void HandleBossHealth(float Normalized);
    // Handle boss phase
    void HandleBossPhase(int32 Phase, const FText& Hint);
};
