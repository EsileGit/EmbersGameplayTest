// Copyright Epic Games, Inc. All Rights Reserved.


#include "EmbersGameplayTestPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "EmbersGameplayTestCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "EmbersGameplayTest.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Widgets/Input/SVirtualJoystick.h"

AEmbersGameplayTestPlayerController::AEmbersGameplayTestPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AEmbersGameplayTestCameraManager::StaticClass();
}

void AEmbersGameplayTestPlayerController::BeginPlay()
{
	Super::BeginPlay();

	
	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogEmbersGameplayTest, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AEmbersGameplayTestPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
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

void AEmbersGameplayTestPlayerController::SetPlayerFrozen(bool bFrozen)
{
	if (ACharacter* PlayerCharacter = GetCharacter())
	{
		if (bFrozen)
		{
			PlayerCharacter->DisableInput(this);
			PlayerCharacter->GetCharacterMovement()->StopMovementImmediately();
		}
		else
		{
			PlayerCharacter->EnableInput(this);
		}
	}

	if (bFrozen)
	{
		SetInputMode(FInputModeUIOnly());
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}

	SetShowMouseCursor(bFrozen);
}

bool AEmbersGameplayTestPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
