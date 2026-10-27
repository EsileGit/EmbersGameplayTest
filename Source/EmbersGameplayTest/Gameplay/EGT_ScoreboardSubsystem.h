// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Gameplay/EGT_ScoreboardSaveGame.h"
#include "EGT_ScoreboardSubsystem.generated.h"

/**
 * Owns the local scoreboards: loads them once per game session (survives level reloads) and saves them on every submit.
 * Rules (board size, which level) are decided by the game mode.
 */
UCLASS()
class EMBERSGAMEPLAYTEST_API UEGT_ScoreboardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Entries of a level's scoreboard, best first (empty if the level has none yet)
	UFUNCTION(BlueprintPure, Category="Scoreboard")
	TArray<FEGT_ScoreboardEntry> GetEntries(FName LevelKey) const;

	// Rank (0 = best) this score would take on the level's scoreboard, or INDEX_NONE if it doesn't make the top MaxEntries
	UFUNCTION(BlueprintPure, Category="Scoreboard")
	int32 GetRankForScore(FName LevelKey, int32 Score, int32 MaxEntries) const;

	// Inserts the run, trims the board to MaxEntries and saves to disk. Returns its rank, or INDEX_NONE if it didn't make it
	UFUNCTION(BlueprintCallable, Category="Scoreboard")
	int32 SubmitScore(FName LevelKey, const FText& LevelDisplayName, const FString& PlayerName, int32 Score, int32 MaxEntries);

protected:
	static const FString SaveSlotName;

	// Longer names are cut
	static constexpr int32 MaxPlayerNameLength = 12;

	UPROPERTY(Transient)
	TObjectPtr<UEGT_ScoreboardSaveGame> SaveGame;
};
