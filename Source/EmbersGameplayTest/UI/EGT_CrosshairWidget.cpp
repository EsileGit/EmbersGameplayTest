// Elise Rosati


#include "UI/EGT_CrosshairWidget.h"

#include "Interaction/InteractionComponent.h"
#include "Component/EGT_ThrowComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CrosshairWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// The crosshair is purely visual: never catch the mouse
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		// The pawn may not be possessed yet (workaround: create an init function in the HUD that inits all widgets when 
		// the player is ready. Could also be used to forward "useful" info to widgets. Would work well with a generic 
		// parent function for all widgets. Bref, a TODO for "later".
		PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::HandlePossessedPawnChanged);
		BindToPawn(PlayerController->GetPawn());
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CrosshairWidget::NativeDestruct()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandlePossessedPawnChanged);
	}

	UnbindFromInteractionComponent();

	Super::NativeDestruct();
}

#pragma region Binding

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CrosshairWidget::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	BindToPawn(NewPawn);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CrosshairWidget::BindToPawn(APawn* Pawn)
{
	UnbindFromInteractionComponent();

	if (!IsValid(Pawn))
	{
		return;
	}

	if (UEGT_ThrowComponent* ThrowComponent = Pawn->FindComponentByClass<UEGT_ThrowComponent>())
	{
		BoundThrowComponent = ThrowComponent;
		ThrowComponent->OnThrowChargeChanged.AddUniqueDynamic(this, &ThisClass::HandleThrowChargeChanged);
		HandleThrowChargeChanged(ThrowComponent->IsCharging());
	}

	UInteractionComponent* InteractionComponent = Pawn->FindComponentByClass<UInteractionComponent>();
	if (!InteractionComponent)
	{
		return;
	}

	BoundInteractionComponent = InteractionComponent;
	InteractionComponent->OnInteractionFocusChanged.AddUniqueDynamic(this, &ThisClass::HandleFocusChanged);
	InteractionComponent->OnHeldItemChanged.AddUniqueDynamic(this, &ThisClass::HandleHeldItemChanged);

	// Push the current state so the Blueprint starts in sync
	HandleFocusChanged(InteractionComponent->GetFocusedActor(), nullptr);
	HandleHeldItemChanged(InteractionComponent->GetHeldActor());
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CrosshairWidget::UnbindFromInteractionComponent()
{
	if (UInteractionComponent* InteractionComponent = BoundInteractionComponent.Get())
	{
		InteractionComponent->OnInteractionFocusChanged.RemoveDynamic(this, &ThisClass::HandleFocusChanged);
		InteractionComponent->OnHeldItemChanged.RemoveDynamic(this, &ThisClass::HandleHeldItemChanged);
	}

	BoundInteractionComponent.Reset();

	if (UEGT_ThrowComponent* ThrowComponent = BoundThrowComponent.Get())
	{
		ThrowComponent->OnThrowChargeChanged.RemoveDynamic(this, &ThisClass::HandleThrowChargeChanged);
	}

	BoundThrowComponent.Reset();
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CrosshairWidget::HandleFocusChanged(AActor* NewFocusedActor, AActor* PreviousFocusedActor)
{
	OnFocusChanged(NewFocusedActor != nullptr, NewFocusedActor);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CrosshairWidget::HandleHeldItemChanged(AActor* HeldItem)
{
	OnHeldStateChanged(HeldItem != nullptr, HeldItem);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CrosshairWidget::HandleThrowChargeChanged(bool bIsCharging)
{
	OnThrowChargeStateChanged(bIsCharging);
}

// ---------------------------------------------------------------------------------------------------------------------
float UEGT_CrosshairWidget::GetThrowChargeAlpha() const
{
	const UEGT_ThrowComponent* ThrowComponent = BoundThrowComponent.Get();
	return ThrowComponent ? ThrowComponent->GetChargeAlpha() : 0.0f;
}

#pragma endregion
