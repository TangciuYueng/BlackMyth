// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
class UButton;
#include "BMMainWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMMainWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    UPROPERTY(meta=(BindWidget)) class UButton* StartButton = nullptr;
    UPROPERTY(meta=(BindWidget)) class UButton* BossBattle = nullptr; // “boss战（我是老手）”
    UPROPERTY(meta=(BindWidget)) class UButton* QuitButton = nullptr;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void OnStartClicked();
    UFUNCTION()
    void OnBossBattleClicked();
    UFUNCTION()
    void OnQuitClicked();
};
