// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UBMEventBusSubsystem;
class UBMUIManagerSubsystem;

#include "BMWidgetBase.generated.h"

UCLASS()
class BLACKMYTH_API UBMWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    // Bind/unbind to the EventBus. Derived widgets override to subscribe their own handlers.
    virtual void BindEventBus(UBMEventBusSubsystem* EventBus);
    virtual void UnbindEventBus(UBMEventBusSubsystem* EventBus);

    UBMEventBusSubsystem* GetEventBus() const;
    UBMUIManagerSubsystem* GetUIManager() const;

private:
    UBMEventBusSubsystem* CachedEventBus = nullptr;
};
