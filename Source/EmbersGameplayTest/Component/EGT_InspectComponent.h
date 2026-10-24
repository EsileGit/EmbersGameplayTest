// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EGT_InspectComponent.generated.h"

class UInteractionComponent;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class APlayerController;

/** Broadcast when inspect mode starts or stops. InspectedActor is null when leaving inspect mode. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInspectModeChanged, bool, bIsInspecting, AActor*, InspectedActor);

/**
 * Inspect mode for the item currently held by the player.
 * While inspecting, the player cannot move or look around.
 * Inspected item can be rotated.
 */
UCLASS(BlueprintType)
class EMBERSGAMEPLAYTEST_API UEGT_InspectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEGT_InspectComponent();

	// Fired when inspect mode starts or stops
	UPROPERTY(BlueprintAssignable, Category="Inspect")
	FOnInspectModeChanged OnInspectModeChanged;

	void ToggleInspect();
	void EnterInspect();
	void ExitInspect();

	// Called when the drag button is pressed/released
	UFUNCTION(BlueprintCallable, Category="Inspect")
	void SetDragging(bool bDragging);

	// Mouse / stick delta forwarded by the owner while inspecting and dragging
	UFUNCTION(BlueprintCallable, Category="Inspect")
	void AddInspectRotationInput(float Yaw, float Pitch);

	// Getter & Setter
	UFUNCTION(BlueprintPure, Category="Inspect")
	bool IsInspecting() const { return bIsInspecting; }
	UFUNCTION(BlueprintPure, Category="Inspect")
	bool IsDragging() const { return bIsDragging; }
	UFUNCTION(BlueprintPure, Category="Inspect")
	AActor* GetInspectedActor() const { return InspectedActor.Get(); }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Inspect

	// Callback triggered by the interaction component
	UFUNCTION()
	void HandleHeldItemChanged(AActor* HeldItem);

	// Input
	void SetInspectMappingContextActive(bool bActive) const;
	void SetInspectCursorActive(bool bActive) const;
	UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem() const;
	APlayerController* GetLocalPlayerController() const;

	// PROPERTIES
	// Mapping context holding the inspect actions, only active while an item is held
	UPROPERTY(EditAnywhere, Category="Inspect|Input")
	TObjectPtr<UInputMappingContext> InspectMappingContext;

	// Above the default contexts
	UPROPERTY(EditAnywhere, Category="Inspect|Input")
	int32 InspectMappingPriority = 10;

	// Where the inspected item sits relative to the camera while inspecting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspect")
	FVector InspectOffset = FVector(35.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspect", meta = (ClampMin = 0))
	float RotationSpeed = 1.0f;

	// Runtime
	// These transforms are read by the BP (allowing a designer to customize the "displace" effect)
	UPROPERTY(BlueprintReadOnly, Category="Inspect", meta=(AllowPrivateAccess))
	FTransform ItemTargetTransform;
	UPROPERTY(BlueprintReadOnly, Category="Inspect", meta=(AllowPrivateAccess))
	FTransform ItemStartTransform;
	FTransform CachedInitialItemTransform;
	
	UPROPERTY(Transient)
	TObjectPtr<UInteractionComponent> InteractionComponent;

	TWeakObjectPtr<AActor> InspectedActor;

	bool bIsInspecting = false;
	bool bIsDragging = false;

#pragma endregion

#pragma region GameFeel

	// Blueprint reactions hook (for cosmetic)
	UFUNCTION(BlueprintImplementableEvent, Category="Inspect")
	void OnInspectStarted(AActor* Item);

	UFUNCTION(BlueprintImplementableEvent, Category="Inspect")
	void OnInspectEnded(AActor* Item);

#pragma endregion
};
