// Copyright Epic Games, Inc. All Rights Reserved.

#include "EmbersGameplayTestCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InteractionComponent.h"
#include "Component/EGT_InspectComponent.h"
#include "Component/EGT_ThrowComponent.h"
#include "EmbersGameplayTest.h"

AEmbersGameplayTestCharacter::AEmbersGameplayTestCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// Create the interaction component that handles looking at and using world objects
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction Component"));

	// For the inspect mode
	InspectComponent = CreateDefaultSubobject<UEGT_InspectComponent>(TEXT("Inspect Component"));

	// For the charged throw
	ThrowComponent = CreateDefaultSubobject<UEGT_ThrowComponent>(TEXT("Throw Component"));

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AEmbersGameplayTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AEmbersGameplayTestCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AEmbersGameplayTestCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEmbersGameplayTestCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEmbersGameplayTestCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AEmbersGameplayTestCharacter::LookInput);

		// Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AEmbersGameplayTestCharacter::DoInteract);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AEmbersGameplayTestCharacter::DoInteractEnd);

		// Inspecting
		EnhancedInputComponent->BindAction(InspectAction, ETriggerEvent::Started, this, &AEmbersGameplayTestCharacter::DoInspect);
		EnhancedInputComponent->BindAction(LeaveInspectMode, ETriggerEvent::Started, this, &AEmbersGameplayTestCharacter::ExitInspect);
		EnhancedInputComponent->BindAction(InspectRotateAction, ETriggerEvent::Started, this, &AEmbersGameplayTestCharacter::DoInspectDragStart);
		EnhancedInputComponent->BindAction(InspectRotateAction, ETriggerEvent::Completed, this, &AEmbersGameplayTestCharacter::DoInspectDragEnd);
	}
	else
	{
		UE_LOG(LogEmbersGameplayTest, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AEmbersGameplayTestCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AEmbersGameplayTestCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AEmbersGameplayTestCharacter::DoAim(float Yaw, float Pitch)
{
	// while inspecting, the look input rotates the held item instead of the camera
	if (IsInspecting())
	{
		if (InspectComponent->IsDragging())
		{
			InspectComponent->AddInspectRotationInput(Yaw, Pitch);
		}
		return;
	}

	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AEmbersGameplayTestCharacter::DoMove(float Right, float Forward)
{
	if (IsInspecting())
	{
		return;
	}

	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AEmbersGameplayTestCharacter::DoJumpStart()
{
	if (IsInspecting())
	{
		return;
	}

	// pass Jump to the character
	Jump();
}

void AEmbersGameplayTestCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void AEmbersGameplayTestCharacter::DoInteract()
{
	// interacting is blocked while inspecting: leave inspect mode first
	if (IsInspecting())
	{
		return;
	}

	// While carrying an item, interacting charges a throw instead of dropping it
	if (InteractionComponent && InteractionComponent->IsHoldingItem() && ThrowComponent)
	{
		ThrowComponent->StartCharging();
		return;
	}

	// pass the interaction to the interaction component
	if (InteractionComponent)
	{
		InteractionComponent->TryInteract();
	}
}

void AEmbersGameplayTestCharacter::DoInteractEnd()
{
	if (ThrowComponent && ThrowComponent->IsCharging())
	{
		ThrowComponent->ReleaseThrow();
	}
}

void AEmbersGameplayTestCharacter::DoInspect()
{
	// inspecting is blocked while charging a throw
	if (ThrowComponent && ThrowComponent->IsCharging())
	{
		return;
	}

	// pass the inspect toggle to the inspect component
	if (InspectComponent)
	{
		InspectComponent->ToggleInspect();
	}
}

void AEmbersGameplayTestCharacter::ExitInspect()
{
	if (InspectComponent)
	{
		InspectComponent->ExitInspect();
	}
}

void AEmbersGameplayTestCharacter::DoInspectDragStart()
{
	if (InspectComponent)
	{
		InspectComponent->SetDragging(true);
	}
}

void AEmbersGameplayTestCharacter::DoInspectDragEnd()
{
	if (InspectComponent)
	{
		InspectComponent->SetDragging(false);
	}
}

bool AEmbersGameplayTestCharacter::IsInspecting() const
{
	return InspectComponent && InspectComponent->IsInspecting();
}
