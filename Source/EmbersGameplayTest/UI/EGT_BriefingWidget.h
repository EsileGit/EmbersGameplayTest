// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EGT_BriefingWidget.generated.h"

/**
 * Rules screen displayed before a game level starts.
 */
UCLASS(Abstract)
class EMBERSGAMEPLAYTEST_API UEGT_BriefingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable, Category="Level Flow")
	void StartLevel();
};
