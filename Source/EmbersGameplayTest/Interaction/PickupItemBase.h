// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "PickupItemBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 *  Base class for world items the player can look at, interact with and carry around.
 *
 *  It only provides the plumbing: an interaction collider on the PickupItem object channel so the
 *  interaction component can find it, a mesh to display, and the Interactable interface wired to a
 *  few editable defaults.
 *
 *  Derive a Blueprint from this class and override the Interactable events to give the item its
 *  actual behaviour.
 */
UCLASS(Abstract, Blueprintable)
class EMBERSGAMEPLAYTEST_API APickupItemBase : public AActor, public IInteractable
{
	GENERATED_BODY()

	/** Collider the interaction trace looks for. Root of the actor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* InteractionCollision;

	/** Visual representation of the item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ItemMesh;

protected:

	/** Text shown on the HUD while the player is looking at this item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
	FText InteractionPrompt;

	/** Whether the player can currently interact with this item at all */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
	bool bInteractionEnabled = true;

	/** Whether interacting with this item makes the player carry it around */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
	bool bCanBeGrabbed = true;

	/** Identifier this item reports to inventory or quest logic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
	FName ItemId;

	/** Radius of the interaction collider */
	UPROPERTY(EditAnywhere, Category="Pickup", meta = (ClampMin = 1, ClampMax = 500, Units = "cm"))
	float InteractionRadius = 40.0f;

public:

	/** Constructor */
	APickupItemBase();

	//~ Begin IInteractable
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanBeGrabbed_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;
	virtual void OnFocusBegin_Implementation(AActor* Interactor) override;
	virtual void OnFocusEnd_Implementation(AActor* Interactor) override;
	//~ End IInteractable

	/** Removes this item from the world. Call it from Blueprint once the item has been consumed. */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void ConsumeItem();

	/** Turns interaction on this item on or off */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void SetInteractionEnabled(bool bEnabled);

	/** Returns the item identifier */
	UFUNCTION(BlueprintPure, Category="Pickup")
	FName GetItemId() const { return ItemId; }

	/** Returns the interaction collider */
	USphereComponent* GetInteractionCollision() const { return InteractionCollision; }

	/** Returns the item mesh */
	UStaticMeshComponent* GetItemMesh() const { return ItemMesh; }

protected:

	/** Native construction script */
	virtual void OnConstruction(const FTransform& Transform) override;
};
