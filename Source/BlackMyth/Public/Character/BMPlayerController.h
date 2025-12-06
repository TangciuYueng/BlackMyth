// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BMPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API ABMPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    void ShowMainMenu();
    void TogglePauseMenu();
};
