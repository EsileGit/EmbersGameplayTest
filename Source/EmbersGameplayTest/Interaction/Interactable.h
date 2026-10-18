// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Common interface for anything the player can focus and interact with.
 *  Implement it in C++ (see APickupItemBase) or directly in a Blueprint's Class Settings.
 */
class EMBERSGAMEPLAYTEST_API IInteractable
{
	GENERATED_BODY()

public:

	/** Returns true if this object is currently available for interaction */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	bool CanInteract(AActor* Interactor);

	/** Runs the interaction for this object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void OnInteract(AActor* Interactor);

	/** Returns true if this object can be picked up and carried around */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	bool CanBeGrabbed(AActor* Interactor);

	/** Returns the text to display on the HUD while this object is focused */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FText GetInteractionPrompt();

	/** Called when the player starts looking at this object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void OnFocusBegin(AActor* Interactor);

	/** Called when the player stops looking at this object */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void OnFocusEnd(AActor* Interactor);
};
