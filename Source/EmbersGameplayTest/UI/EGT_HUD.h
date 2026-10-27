// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EmbersGameplayTestGameMode.h"
#include "EGT_HUD.generated.h"

class UUserWidget;
class UEGT_BriefingWidget;
class UEGT_ResultsWidget;

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
	virtual void DrawHUD() override;

#pragma region Debug

	// Lists the game mode's requests and their state on screen (toggled by EGT.Debug.Requests)
	void DrawRequestsDebug();

	// PROPERTIES
	// Top-left corner of the requests debug list, in screen pixels
	UPROPERTY(EditAnywhere, Category="Debug")
	FVector2D RequestsDebugOrigin = FVector2D(20.0f, 150.0f);

	// Space kept free below the requests debug list, in screen pixels
	UPROPERTY(EditAnywhere, Category="Debug")
	float RequestsDebugBottomMargin = 20.0f;

	// Runtime
	// First request shown by the debug list, scrolled with PageUp / PageDown
	int32 RequestsDebugFirstIndex = 0;

#pragma endregion

#pragma region LevelFlow

	UFUNCTION()
	void HandleLevelPhaseChanged(EEGT_LevelPhase NewPhase);

	// PROPERTIES
	// Rules screen, displayed until the level starts
	UPROPERTY(EditAnywhere, Category="UI|Level Flow")
	TSubclassOf<UEGT_BriefingWidget> BriefingWidgetClass;

	// Score screen, displayed when the level timer runs out
	UPROPERTY(EditAnywhere, Category="UI|Level Flow")
	TSubclassOf<UEGT_ResultsWidget> ResultsWidgetClass;

	// Runtime
	UPROPERTY(Transient)
	TObjectPtr<UEGT_BriefingWidget> BriefingWidget;

	UPROPERTY(Transient)
	TObjectPtr<UEGT_ResultsWidget> ResultsWidget;

#pragma endregion

	// PROPERTIES
	// Root overlay widget
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> OverlayWidgetClass;

	// Runtime
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> OverlayWidget;
};
