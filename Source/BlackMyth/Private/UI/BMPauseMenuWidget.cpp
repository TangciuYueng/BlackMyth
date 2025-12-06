// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMPauseMenuWidget.h"
#include "Components/Button.h"
#include "System/Event/BMEventBusSubsystem.h"

void UBMPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnResumeClicked);
    }
    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnSettingsClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UBMPauseMenuWidget::OnQuitClicked);
    }
}

void UBMPauseMenuWidget::NativeDestruct()
{
    if (ResumeButton)
    {
        ResumeButton->OnClicked.RemoveAll(this);
    }
    if (SettingsButton)
    {
        SettingsButton->OnClicked.RemoveAll(this);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UBMPauseMenuWidget::OnResumeClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(NSLOCTEXT("BM", "Resume", "Resuming game"));
    }
}

void UBMPauseMenuWidget::OnSettingsClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(NSLOCTEXT("BM", "Settings", "Open Settings"));
    }
}

void UBMPauseMenuWidget::OnQuitClicked()
{
    if (UBMEventBusSubsystem* Bus = GetEventBus())
    {
        Bus->EmitNotify(NSLOCTEXT("BM", "Quit", "Quit to main menu"));
    }
}

