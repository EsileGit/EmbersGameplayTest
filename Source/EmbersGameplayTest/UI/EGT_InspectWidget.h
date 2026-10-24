// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/EGT_ParcelBase.h"
#include "EGT_InspectWidget.generated.h"

class APawn;
class UEGT_InspectComponent;

/**
 * Base class for the panel displayed while inspecting a held item.
 */
UCLASS(Abstract)
class EMBERSGAMEPLAYTEST_API UEGT_InspectWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

#pragma region Inspect

	/** Blueprint reactions hook (for cosmetic) */
	// Called when the player starts inspecting an item. Info is left default if the item is not a parcel.
	UFUNCTION(BlueprintImplementableEvent, Category="Inspect")
	void OnInspectStarted(AActor* InspectedActor, const FEGT_ParcelInspectInfo& Info);

	// Called when the player stops inspecting.
	UFUNCTION(BlueprintImplementableEvent, Category="Inspect")
	void OnInspectEnded();

#pragma endregion

#pragma region Binding

	// Callback triggered by the owning controller, used to rebind on the new pawn
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	// Callback triggered by the pawn's inspect component
	UFUNCTION()
	void HandleInspectModeChanged(bool bIsInspecting, AActor* InspectedActor);

	// Unbinds from the current inspect component (if any) and binds to the one on the given pawn
	void BindToPawn(APawn* Pawn);
	void UnbindFromInspectComponent();

	// Runtime
	TWeakObjectPtr<UEGT_InspectComponent> BoundInspectComponent;

#pragma endregion
};
