// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/PickupItemBase.h"
#include "EGT_PickableParcelBase.generated.h"

class UInteractionComponent;
/**
 * /!\ OBSOLETE /!\ (Abandonned for ease of use and gain of time)
 * A parcel item that derives from the PickupItem
 */
UCLASS()
class EMBERSGAMEPLAYTEST_API AEGT_PickableParcelBase : public APickupItemBase
{
	GENERATED_BODY()
	
public:
	AEGT_PickableParcelBase();
	
	
	//~ Begin IInteractable
	void OnInteract_Implementation(AActor* Interactor) override;
	
protected:
#pragma region Interaction
	
	// Callback triggered by the player's itneraction component
	UFUNCTION()
	void HandleHeldItemChanged(AActor* HeldItem);
	
	UPROPERTY(Transient)
	TObjectPtr<UInteractionComponent> HoldingInteractionComponent;
#pragma endregion
};
