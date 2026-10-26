// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "EGT_SnapSpotComponent.generated.h"

class AEGT_ParcelBase;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParcelSnapped, AEGT_ParcelBase*, Parcel);

/**
 * Spot where a parcel snaps when it enters the snap zone: the parcel's snap attach point is placed on this arrow.
 * Holds one parcel at a time. 
 */
UCLASS(ClassGroup=(Parcel), meta=(BlueprintSpawnableComponent))
class EMBERSGAMEPLAYTEST_API UEGT_SnapSpotComponent : public UArrowComponent
{
	GENERATED_BODY()

public:
	UEGT_SnapSpotComponent();

	UFUNCTION(BlueprintPure, Category="Snap")
	AEGT_ParcelBase* GetOccupant() const { return Occupant.Get(); }

	// Fired after a parcel has been snapped on this spot
	UPROPERTY(BlueprintAssignable, Category="Snap")
	FOnParcelSnapped OnParcelSnapped;

protected:
	virtual void OnRegister() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleSnapZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleSnapZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// Whether a parcel still lies on this spot
	bool IsOccupied() const;

	// PROPERTIES
	// Half size of the zone that triggers the snap. Make it stick out of the shelf so that thrown parcels snap before hitting it
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Snap|Zone", meta = (Units = "cm"))
	FVector SnapZoneExtent = FVector(10.0f, 40.0f, 40.0f);

	// Snap zone position, relative to the arrow
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Snap|Zone", meta = (Units = "cm"))
	FVector SnapZoneOffset = FVector(0.0f, 0.0f, 30.0f);

	// Runtime
	// Created on register (not a Blueprint component) so that a snap spot is a single component to place
	UPROPERTY(Transient)
	TObjectPtr<UBoxComponent> SnapZone;

	UPROPERTY(Transient, VisibleInstanceOnly, Category="Snap")
	TWeakObjectPtr<AEGT_ParcelBase> Occupant;
};
