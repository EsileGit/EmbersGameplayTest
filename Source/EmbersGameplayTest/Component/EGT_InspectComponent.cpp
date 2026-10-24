// Elise Rosati


#include "Component/EGT_InspectComponent.h"

#include "EmbersGameplayTest.h"
#include "InteractionComponent.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

// ---------------------------------------------------------------------------------------------------------------------
UEGT_InspectComponent::UEGT_InspectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::BeginPlay()
{
	Super::BeginPlay();

	InteractionComponent = GetOwner()->FindComponentByClass<UInteractionComponent>();
	if (!InteractionComponent)
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("'%s' has no InteractionComponent: inspect mode will never be available."), *GetNameSafe(GetOwner()));
		return;
	}

	InteractionComponent->OnHeldItemChanged.AddUniqueDynamic(this, &ThisClass::HandleHeldItemChanged);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InteractionComponent)
	{
		InteractionComponent->OnHeldItemChanged.RemoveDynamic(this, &ThisClass::HandleHeldItemChanged);
	}

	SetInspectMappingContextActive(false);

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// This safeguard is uselss for now (as nothing should be destroying the parcel during inspect)
	if (!InspectedActor.IsValid())
	{
		ExitInspect();
		SetInspectMappingContextActive(false);
	}
}

#pragma region Inspect

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::ToggleInspect()
{
	if (bIsInspecting)
	{
		ExitInspect();
	}
	else
	{
		EnterInspect();
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::EnterInspect()
{
	if (bIsInspecting || !InteractionComponent || !InteractionComponent->IsHoldingItem())
	{
		return;
	}

	UE_LOG(LogEmbersGameplayTest, Log, TEXT("Entering Inspect mode"));
	
	AActor* Item = InteractionComponent->GetHeldActor();

	bIsInspecting = true;
	bIsDragging = false;
	InspectedActor = Item;
	SetComponentTickEnabled(true);

	// This is sensitive to a returning object that gets inspected before reaching back its initial position... TODO: use 
	// a bool to decide whether or not to rewrite this
	CachedInitialItemTransform = Item->GetRootComponent()->GetRelativeTransform();
	ItemStartTransform = CachedInitialItemTransform;
	ItemTargetTransform = ItemStartTransform;
	ItemTargetTransform.SetLocation(ItemTargetTransform.GetLocation() + InspectOffset);

	SetInspectCursorActive(true);

	OnInspectStarted(Item);
	OnInspectModeChanged.Broadcast(true, Item);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::ExitInspect()
{
	if (!bIsInspecting)
	{
		return;
	}

	UE_LOG(LogEmbersGameplayTest, Log, TEXT("Exiting Inspect mode"));
	
	AActor* Item = InspectedActor.Get();

	bIsInspecting = false;
	bIsDragging = false;
	InspectedActor.Reset();
	SetComponentTickEnabled(false);

	ItemStartTransform = Item->GetRootComponent()->GetRelativeTransform();
	ItemTargetTransform = CachedInitialItemTransform;

	SetInspectCursorActive(false);

	OnInspectEnded(Item);
	OnInspectModeChanged.Broadcast(false, nullptr);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::SetDragging(bool bDragging)
{
	bIsDragging = bIsInspecting && bDragging;
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::AddInspectRotationInput(float Yaw, float Pitch)
{
	if (!bIsInspecting || !bIsDragging || !InspectedActor.IsValid())
	{
		return;
	}
	
	UE_LOG(LogEmbersGameplayTest, Verbose, TEXT("Rotating object in Inspect mode [%.1f, %.1f]"), Yaw, Pitch);
	
	const FQuat YawDelta(FVector::UpVector, FMath::DegreesToRadians(Yaw * RotationSpeed));
	const FQuat PitchDelta(FVector::RightVector, FMath::DegreesToRadians(Pitch * RotationSpeed));
	
	//InspectedActor->GetRootComponent()->AddLocalRotation(YawDelta * PitchDelta);
	UE_LOG(LogEmbersGameplayTest, Verbose, TEXT("Old rotation: [%s]"), *InspectedActor->GetRootComponent()->GetRelativeRotation().ToCompactString());
	InspectedActor->GetRootComponent()->SetRelativeRotation((YawDelta * PitchDelta) * InspectedActor->GetRootComponent()->GetRelativeRotation().Quaternion());
	UE_LOG(LogEmbersGameplayTest, Verbose, TEXT("New rotation: [%s]"), *InspectedActor->GetRootComponent()->GetRelativeRotation().ToCompactString());
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::HandleHeldItemChanged(AActor* HeldItem)
{
	// Inspect actions are only available while something is held (this allows for reuse of same key and reduces the 
	// risk at overlapping undesired behaviors)
	SetInspectMappingContextActive(HeldItem != nullptr);

	// The inspected item has been dropped, swapped or consumed
	if (bIsInspecting && HeldItem != InspectedActor.Get())
	{
		ExitInspect();
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::SetInspectMappingContextActive(bool bActive) const
{
	if (!InspectMappingContext)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetInputSubsystem();
	if (!Subsystem)
	{
		return;
	}

	if (bActive)
	{
		Subsystem->AddMappingContext(InspectMappingContext, InspectMappingPriority);
	}
	else
	{
		Subsystem->RemoveMappingContext(InspectMappingContext);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectComponent::SetInspectCursorActive(bool bActive) const
{
	APlayerController* PlayerController = GetLocalPlayerController();
	if (!PlayerController)
	{
		return;
	}

	if (bActive)
	{
		// Game and UI: the cursor is free, but clicks still reach the game (the inspect widget is hit test invisible).
		// While a button is held the viewport captures the mouse: the cursor is hidden and mouse deltas keep flowing
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);
		InputMode.SetHideCursorDuringCapture(true);
		PlayerController->SetInputMode(InputMode);
	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	PlayerController->SetShowMouseCursor(bActive);
}

// ---------------------------------------------------------------------------------------------------------------------
UEnhancedInputLocalPlayerSubsystem* UEGT_InspectComponent::GetInputSubsystem() const
{
	const APlayerController* PlayerController = GetLocalPlayerController();
	if (!PlayerController)
	{
		return nullptr;
	}

	return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
}

// ---------------------------------------------------------------------------------------------------------------------
APlayerController* UEGT_InspectComponent::GetLocalPlayerController() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PlayerController = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	if (!PlayerController || !PlayerController->IsLocalPlayerController())
	{
		return nullptr;
	}

	return PlayerController;
}

#pragma endregion
