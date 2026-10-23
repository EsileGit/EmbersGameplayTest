// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EGT_HUD.generated.h"

class UUserWidget;

/**
 * HUD class to bear all UI-related elements (more centralized than the PC)
 */
UCLASS(Abstract)
class EMBERSGAMEPLAYTEST_API AEGT_HUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// PROPERTIES
	// Root overlay widget
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> OverlayWidgetClass;

	// Runtime
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> OverlayWidget;
};
