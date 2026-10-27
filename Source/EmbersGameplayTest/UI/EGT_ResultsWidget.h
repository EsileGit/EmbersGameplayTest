// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/EGT_ScoreboardSaveGame.h"
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
	// Called by the HUD right after the widget is displayed.
	// QualifyingRank: rank (0 = best) the score would take on the scoreboard, -1 if it doesn't make it (no name to ask)
	UFUNCTION(BlueprintImplementableEvent, Category="Level Flow")
	void OnShowResults(int32 Score, int32 QualifyingRank);

protected:
	// Reloads the current level from scratch
	UFUNCTION(BlueprintCallable, Category="Level Flow")
	void RestartLevel();

	// Player-facing name of the current level
	UFUNCTION(BlueprintPure, Category="Level Flow")
	FText GetLevelDisplayName() const;

	// Records the score under this name. Returns its rank (0 = best), or -1 if not recorded
	UFUNCTION(BlueprintCallable, Category="Level Flow|Scoreboard")
	int32 SubmitPlayerName(const FString& PlayerName);

	// The current level's scoreboard, best first
	UFUNCTION(BlueprintPure, Category="Level Flow|Scoreboard")
	TArray<FEGT_ScoreboardEntry> GetScoreboardEntries() const;
};
