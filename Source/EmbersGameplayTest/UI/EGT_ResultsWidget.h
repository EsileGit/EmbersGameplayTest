// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EGT_ResultsWidget.generated.h"

/**
 * Results screen displayed when the level timer runs out.
 */
UCLASS(Abstract)
class EMBERSGAMEPLAYTEST_API UEGT_ResultsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Blueprint reactions hook (for cosmetic) */
	// Called by the HUD right after the widget is displayed
	UFUNCTION(BlueprintImplementableEvent, Category="Level Flow")
	void OnShowResults(int32 Score);

protected:
	// Reloads the current level from scratch
	UFUNCTION(BlueprintCallable, Category="Level Flow")
	void RestartLevel();
};
