// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UProgressBar;
class UTextBlock;
#include "BMBossBarBase.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMBossBarBase : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    UPROPERTY(meta=(BindWidget)) class UProgressBar* BossHealthBar = nullptr;
    UPROPERTY(meta=(BindWidget)) class UTextBlock* PhaseHintText = nullptr;

    virtual void NativeConstruct() override;

protected:
    virtual void BindEventBus(class UBMEventBusSubsystem* EventBus) override;
    virtual void UnbindEventBus(class UBMEventBusSubsystem* EventBus) override;

private:
    FDelegateHandle BossHealthHandle;
    FDelegateHandle BossPhaseHandle;

    void HandleBossHealth(float Normalized);
    void HandleBossPhase(int32 Phase, const FText& Hint);
};
