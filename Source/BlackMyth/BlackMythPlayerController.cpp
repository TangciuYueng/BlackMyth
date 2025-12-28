// Copyright Epic Games, Inc. All Rights Reserved.


#include "BlackMythPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "BlackMyth.h"
#include "Widgets/Input/SVirtualJoystick.h"

#include "InputCoreTypes.h"
#include "Character/BMPlayerCharacter.h"

/*
 * @brief Constructor for the BlackMythPlayerController class
 * @return void
 */
void ABlackMythPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogBlackMyth, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

/* 
 * @brief Setup the input component
 * @return void
 */
void ABlackMythPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ABlackMythPlayerController::Hotbar1);
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ABlackMythPlayerController::Hotbar2);
		InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ABlackMythPlayerController::Hotbar3);
		InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ABlackMythPlayerController::Hotbar4);
		InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ABlackMythPlayerController::Hotbar5);
		InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &ABlackMythPlayerController::Hotbar6);
		InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &ABlackMythPlayerController::Hotbar7);
		InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &ABlackMythPlayerController::Hotbar8);
		InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ABlackMythPlayerController::Hotbar9);
	}

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

/*
 * @brief Hotbar 1
 * @return void
 */
void ABlackMythPlayerController::Hotbar1() { TriggerHotbar(1); }

/*
 * @brief Hotbar 2
 * @return void
 */
void ABlackMythPlayerController::Hotbar2() { TriggerHotbar(2); }

/*
 * @brief Hotbar 3
 * @return void
 */
void ABlackMythPlayerController::Hotbar3() { TriggerHotbar(3); }
void ABlackMythPlayerController::Hotbar4() { TriggerHotbar(4); }
void ABlackMythPlayerController::Hotbar5() { TriggerHotbar(5); }
void ABlackMythPlayerController::Hotbar6() { TriggerHotbar(6); }
void ABlackMythPlayerController::Hotbar7() { TriggerHotbar(7); }
void ABlackMythPlayerController::Hotbar8() { TriggerHotbar(8); }
void ABlackMythPlayerController::Hotbar9() { TriggerHotbar(9); }

/*
 * @brief Trigger the hotbar
 * @param SlotIndex The slot index
 * @return void
 */
void ABlackMythPlayerController::TriggerHotbar(int32 SlotIndex)
{
	if (ABMPlayerCharacter* BMPlayer = Cast<ABMPlayerCharacter>(GetPawn()))
	{
		UE_LOG(LogBlackMyth, Log, TEXT("PC Hotbar %d"), SlotIndex);
		BMPlayer->TriggerHotbarSlot(SlotIndex);
	}
}

/*
 * @brief Should use touch controls
 * @return bool
 */
bool ABlackMythPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
