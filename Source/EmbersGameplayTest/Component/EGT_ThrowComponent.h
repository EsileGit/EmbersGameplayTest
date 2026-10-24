// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EGT_ThrowComponent.generated.h"

class UInteractionComponent;
class UCameraComponent;
class UCurveFloat;

/** Broadcast when the throw charge starts or stops (released or cancelled). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThrowChargeChanged, bool, bIsCharging);

/**
 * Charged throw for the item currently held by the player.
 * While charging, the gauge ping-pongs between 0 and 1, for a more "skill-based" gameplay.
 * Throw is mass dependent: heavy items fly less than light ones (the idea is to really put the throw mechanic at the center).
 */
UCLASS(BlueprintType, Blueprintable)
class EMBERSGAMEPLAYTEST_API UEGT_ThrowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEGT_ThrowComponent();

	// Fired when the charge starts or stops
	UPROPERTY(BlueprintAssignable, Category="Throw")
	FOnThrowChargeChanged OnThrowChargeChanged;

	UFUNCTION(BlueprintCallable, Category="Throw")
	void StartCharging();
	UFUNCTION(BlueprintCallable, Category="Throw")
	void ReleaseThrow();
	UFUNCTION(BlueprintCallable, Category="Throw")
	void CancelCharge();

	// Getter & Setter
	UFUNCTION(BlueprintPure, Category="Throw")
	bool IsCharging() const { return bIsCharging; }
	// Current gauge value, in range [0, 1]
	UFUNCTION(BlueprintPure, Category="Throw")
	float GetChargeAlpha() const { return ChargeAlpha; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Throw

	UFUNCTION()
	void HandleHeldItemChanged(AActor* HeldItem);

	void UpdateChargeAlpha();
	void ApplyThrow(AActor* Item, float Strength) const;
	void StopCharging();

	// PROPERTIES
	// Time for the gauge to go from empty to full (a full ping-pong cycle takes twice as long)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Gauge", meta = (ClampMin = 0.05, Units = "s"))
	float ChargeDuration = 0.8f;

	// Optional remap of the gauge value (X and Y in [0, 1])
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Gauge")
	TObjectPtr<UCurveFloat> ChargeCurve;

	// Min throw speed at an empty gauge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Strength", meta = (ClampMin = 0, Units = "CentimetersPerSecond"))
	float MinThrowSpeed = 150.0f;

	// Max throw speed at a full gauge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Strength", meta = (ClampMin = 0, Units = "CentimetersPerSecond"))
	float MaxThrowSpeed = 1500.0f;

	// Raises the throw direction above the camera forward
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Strength", meta = (ClampMin = -45, ClampMax = 45, Units = "Degrees"))
	float ThrowPitchOffset = 5.0f;

	// Range of masses to remap the thrown item's mass, so very heavy / very light parcels remain playable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Mass", meta = (ClampMin = 0.01, Units = "kg"))
	float LightMassKg = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throw|Mass", meta = (ClampMin = 0.01, Units = "kg"))
	float HeavyMassKg = 10.0f;

	// Runtime
	UPROPERTY(Transient)
	TObjectPtr<UInteractionComponent> InteractionComponent;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> CameraComponent;

	float ChargeTime = 0.0f;
	float ChargeAlpha = 0.0f;
	bool bIsCharging = false;

	// Set while ReleaseThrow drops the item, so the drop isn't mistaken for a cancel
	bool bIsReleasing = false;

#pragma endregion

#pragma region GameFeel

	/** Blueprint reactions hook (for cosmetic) */
	UFUNCTION(BlueprintImplementableEvent, Category="Throw")
	void OnThrowChargeStarted(AActor* Item);

	UFUNCTION(BlueprintImplementableEvent, Category="Throw")
	void OnThrowReleased(AActor* Item, float Strength);

	UFUNCTION(BlueprintImplementableEvent, Category="Throw")
	void OnThrowCancelled(AActor* Item);

#pragma endregion
};
