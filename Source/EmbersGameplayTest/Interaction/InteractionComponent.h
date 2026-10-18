// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UCameraComponent;

/** Broadcast when the actor under the player's crosshair changes. Either parameter may be null. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionFocusChanged, AActor*, NewFocusedActor, AActor*, PreviousFocusedActor);

/** Broadcast when the player picks up or drops a carried item. The parameter is null when nothing is carried. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeldItemChanged, AActor*, HeldItem);

/**
 *  First person interaction component.
 *
 *  Traces from the owning pawn's camera every frame to find the interactable actor the player is
 *  looking at, raises focus events for the HUD, and runs the interaction when the player presses
 *  the interact input. Interactables that report themselves as grabbable are attached to the camera
 *  and carried until the player interacts again.
 *
 *  Add it to a first person character and drive it from an Enhanced Input action.
 */
UCLASS(ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class EMBERSGAMEPLAYTEST_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Fired when the focused actor changes. Bind from the HUD to show or hide the interaction prompt. */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractionFocusChanged OnInteractionFocusChanged;

	/** Fired when the carried item changes. */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnHeldItemChanged OnHeldItemChanged;

protected:

	/** How far in front of the camera we look for interactables */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction", meta = (ClampMin = 0, ClampMax = 2000, Units = "cm"))
	float InteractionDistance = 250.0f;

	/** Where a carried item sits relative to the camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FVector HeldItemOffset = FVector(60.0f, 0.0f, -10.0f);

	/** Draws the interaction trace in the world. Debug only. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug")
	bool bDrawDebugTrace = false;

	/** Camera we trace from. Cached from the owning pawn on BeginPlay. */
	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> CameraComponent;

	/** Actor currently under the crosshair */
	AActor* FocusedActor = nullptr;

	/** Item the player is currently carrying, if any */
	UPROPERTY(Transient)
	TObjectPtr<AActor> HeldActor;

public:

	/** Constructor */
	UInteractionComponent();

	/** Runs the interaction on the focused actor, or drops the carried item if there is one */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void TryInteract();

	/** Releases the carried item back into the world */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void DropHeldItem();

	/** Returns the actor currently under the crosshair, if any */
	UFUNCTION(BlueprintPure, Category="Interaction")
	AActor* GetFocusedActor() const { return FocusedActor; }

	/** Returns the item the player is carrying, if any */
	UFUNCTION(BlueprintPure, Category="Interaction")
	AActor* GetHeldActor() const { return HeldActor; }

	/** Returns true while the player is carrying an item */
	UFUNCTION(BlueprintPure, Category="Interaction")
	bool IsHoldingItem() const { return HeldActor != nullptr; }

	/** Returns the prompt text of the focused actor, or an empty text if nothing is focused */
	UFUNCTION(BlueprintPure, Category="Interaction")
	FText GetFocusedInteractionPrompt() const;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Refreshes the focused actor every frame */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Runs the interaction trace and refreshes the focused actor */
	void UpdateFocus();

	/** Traces forward from the camera looking for an interactable. Returns true on a hit. */
	bool TraceForInteractable(FHitResult& OutHit) const;

	/** Swaps the focused actor and raises the focus events */
	void SetFocusedActor(AActor* NewFocusedActor);

	/** Attaches an interactable to the camera so the player carries it around */
	void GrabItem(AActor* ItemToGrab);
};
