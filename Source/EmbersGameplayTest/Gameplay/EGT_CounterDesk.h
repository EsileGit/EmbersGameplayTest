// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/EGT_RequestTypes.h"
#include "EGT_CounterDesk.generated.h"

class AEGT_ParcelBase;
class AEmbersGameplayTestGameMode;
class UBoxComponent;
class UWidgetComponent;

/** Broadcast when a request starts or stops being pending on the desk */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskRequestChanged, bool, bHasActiveRequest);

/**
 * Counter where requests are drafted, cancelled and delivered. Holds at most one request at a time.
 */
UCLASS(Abstract, Blueprintable)
class EMBERSGAMEPLAYTEST_API AEGT_CounterDesk : public AActor
{
	GENERATED_BODY()

public:
	AEGT_CounterDesk();

	// Drafts a new request if none is pending on this desk
	UFUNCTION(BlueprintCallable, Category="Desk")
	bool TryDraftRequest();

	// Marks the pending request as failed (0 points) and frees the desk
	UFUNCTION(BlueprintCallable, Category="Desk")
	bool CancelActiveRequest();

	UFUNCTION(BlueprintPure, Category="Desk")
	bool HasActiveRequest() const { return ActiveRequestIndex != INDEX_NONE; }

	// Returns false if no request is pending on this desk
	UFUNCTION(BlueprintPure, Category="Desk")
	bool GetActiveRequest(FEGT_Request& OutRequest) const;

	// Real-time state of the pending request. Returns false if no request is pending on this desk
	UFUNCTION(BlueprintPure, Category="Desk")
	bool GetActiveRequestProgress(float& OutRemainingTime, int32& OutCurrentPoints) const;

	// Fired after a request is drafted, cancelled or fulfilled
	UPROPERTY(BlueprintAssignable, Category="Desk")
	FOnDeskRequestChanged OnActiveRequestChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Root;

	// Parcels entering this zone are checked against the active request
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> DeliveryZone;

	AEmbersGameplayTestGameMode* GetGameMode() const;

#pragma region Delivery

	UFUNCTION()
	void HandleDeliveryZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void DeliverParcel(AEGT_ParcelBase* Parcel);

	// PROPERTIES
	// Time the accepted parcel stays in the world before being destroyed (lets Blueprint animate it). 0 = immediately
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Desk|Delivery", meta = (ClampMin = 0, Units = "s"))
	float ConsumeDelay = 0.0f;

	// Runtime
	// Index of the pending request in the game mode's requests
	UPROPERTY(Transient, VisibleInstanceOnly, Category="Desk")
	int32 ActiveRequestIndex = INDEX_NONE;

#pragma endregion

#pragma region GameFeel

	/** Blueprint reactions hook (for cosmetic) */
	UFUNCTION(BlueprintImplementableEvent, Category="Desk")
	void OnRequestDrafted(const FEGT_Request& Request);

	// The draft button was pressed but every request is already done or taken
	UFUNCTION(BlueprintImplementableEvent, Category="Desk")
	void OnNoRequestAvailable();

	UFUNCTION(BlueprintImplementableEvent, Category="Desk")
	void OnRequestCancelled(const FEGT_Request& Request);

	// The parcel doesn't match the request
	UFUNCTION(BlueprintImplementableEvent, Category="Desk")
	void OnParcelRejected(AEGT_ParcelBase* Parcel, const FEGT_Request& Request);

	// Called before the parcel is consumed
	UFUNCTION(BlueprintImplementableEvent, Category="Desk")
	void OnRequestFulfilled(AEGT_ParcelBase* Parcel, const FEGT_Request& Request, int32 Points);

#pragma endregion
};
