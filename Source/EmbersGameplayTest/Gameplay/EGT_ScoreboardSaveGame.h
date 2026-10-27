// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EGT_ScoreboardSaveGame.generated.h"

/** One finished run on a level's scoreboard */
USTRUCT(BlueprintType)
struct FEGT_ScoreboardEntry
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadOnly, Category="Scoreboard")
	FString PlayerName;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category="Scoreboard")
	int32 Score = 0;

	// Display name the level had when the run was submitted (the board itself is keyed by the level asset name)
	UPROPERTY(SaveGame, BlueprintReadOnly, Category="Scoreboard")
	FText LevelDisplayName;

	// When the run was submitted (local time)
	UPROPERTY(SaveGame, BlueprintReadOnly, Category="Scoreboard")
	FDateTime Date;
};

/** Best runs of a level, sorted from best to worst. Wrapper because a TMap value can't be a TArray in a UPROPERTY */
USTRUCT()
struct FEGT_LevelScoreboard
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FEGT_ScoreboardEntry> Entries;
};

/**
 * Local scoreboards of every level, persisted on disk (see UEGT_ScoreboardSubsystem)
 */
UCLASS()
class EMBERSGAMEPLAYTEST_API UEGT_ScoreboardSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Map: level asset name -> its scoreboard
	UPROPERTY(SaveGame)
	TMap<FName, FEGT_LevelScoreboard> Boards;
};
