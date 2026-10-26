// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/EGT_RequestTypes.h"
#include "EGT_CounterDeskWidget.generated.h"

class AEGT_CounterDesk;

/**
 * Widget attached to a counter desk, depicting its pending request (or its lack thereof).
 */
UCLASS(Abstract)
class EMBERSGAMEPLAYTEST_API UEGT_CounterDeskWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Desk")
	void SetCounterDesk(AEGT_CounterDesk* Desk);

protected:
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

#pragma region Desk

	/** Blueprint reactions hook (for cosmetic) */
	UFUNCTION(BlueprintImplementableEvent, Category="Desk")
	void OnRequestStarted(const FEGT_Request& Request);

	UFUNCTION(BlueprintImplementableEvent, Category="Desk")
	void OnRequestCleared();

	UFUNCTION(BlueprintImplementableEvent, Category="Desk")
	void OnRequestProgress(float RemainingTime, float RemainingTimeAlpha, int32 CurrentPoints);

#pragma endregion

#pragma region Binding

	// Callback triggered by the bound desk
	UFUNCTION()
	void HandleActiveRequestChanged(bool bHasActiveRequest);

	void UnbindFromDesk();

	// Runtime
	TWeakObjectPtr<AEGT_CounterDesk> BoundDesk;

	// Max time of the pending request, cached to compute RemainingTimeAlpha
	float ActiveRequestMaxTime = 0.0f;

#pragma endregion
};
