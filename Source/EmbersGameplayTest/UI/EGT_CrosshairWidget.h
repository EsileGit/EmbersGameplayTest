// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EGT_CrosshairWidget.generated.h"

class APawn;
class UInteractionComponent;

/**
 * Base class for the screen-centered crosshair.
 */
UCLASS(Abstract)
class EMBERSGAMEPLAYTEST_API UEGT_CrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

#pragma region Crosshair

	/** Blueprint reactions hook (for cosmetic) */
	// Called when an interactable starts or stops being focused.
	UFUNCTION(BlueprintImplementableEvent, Category="Crosshair")
	void OnFocusChanged(bool bHasFocus, AActor* FocusedActor);

	// Called when the player picks up or drops an item.
	UFUNCTION(BlueprintImplementableEvent, Category="Crosshair")
	void OnHeldStateChanged(bool bIsHolding, AActor* HeldItem);

#pragma endregion

#pragma region Binding

	// Callback triggered by the owning controller, used to rebind on the new pawn
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	// Callbacks triggered by the pawn's interaction component
	UFUNCTION()
	void HandleFocusChanged(AActor* NewFocusedActor, AActor* PreviousFocusedActor);

	UFUNCTION()
	void HandleHeldItemChanged(AActor* HeldItem);

	// Unbinds from the current interaction component (if any) and binds to the one on the given pawn
	void BindToPawn(APawn* Pawn);
	void UnbindFromInteractionComponent();

	// Runtime
	TWeakObjectPtr<UInteractionComponent> BoundInteractionComponent;

#pragma endregion
};
