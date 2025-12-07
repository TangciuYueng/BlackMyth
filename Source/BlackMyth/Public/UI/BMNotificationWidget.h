// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UVerticalBox;
#include "BMNotificationWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMNotificationWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    UPROPERTY(meta=(BindWidget)) class UVerticalBox* MessageList = nullptr;

protected:
    virtual void BindEventBus(class UBMEventBusSubsystem* EventBus) override;
    virtual void UnbindEventBus(class UBMEventBusSubsystem* EventBus) override;

private:
    FDelegateHandle NotifyHandle;
    void HandleNotify(const FText& Msg);
};
