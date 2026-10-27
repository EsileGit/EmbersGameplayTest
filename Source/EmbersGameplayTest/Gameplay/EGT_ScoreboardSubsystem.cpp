// Elise Rosati


#include "Gameplay/EGT_ScoreboardSubsystem.h"

#include "EmbersGameplayTest.h"

#include "Kismet/GameplayStatics.h"

const FString UEGT_ScoreboardSubsystem::SaveSlotName = TEXT("Scoreboard");

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ScoreboardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SaveGame = Cast<UEGT_ScoreboardSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (!SaveGame)
	{
		SaveGame = Cast<UEGT_ScoreboardSaveGame>(UGameplayStatics::CreateSaveGameObject(UEGT_ScoreboardSaveGame::StaticClass()));
	}
}

// ---------------------------------------------------------------------------------------------------------------------
TArray<FEGT_ScoreboardEntry> UEGT_ScoreboardSubsystem::GetEntries(FName LevelKey) const
{
	const FEGT_LevelScoreboard* Board = SaveGame->Boards.Find(LevelKey);
	return Board ? Board->Entries : TArray<FEGT_ScoreboardEntry>();
}

// ---------------------------------------------------------------------------------------------------------------------
int32 UEGT_ScoreboardSubsystem::GetRankForScore(FName LevelKey, int32 Score, int32 MaxEntries) const
{
	const FEGT_LevelScoreboard* Board = SaveGame->Boards.Find(LevelKey);
	const TArray<FEGT_ScoreboardEntry> NoEntries;
	const TArray<FEGT_ScoreboardEntry>& Entries = Board ? Board->Entries : NoEntries;

	// Strictly better only: on a tie, the older run keeps its place
	int32 Rank = 0;
	while (Rank < Entries.Num() && Entries[Rank].Score >= Score)
	{
		++Rank;
	}
	return Rank < MaxEntries ? Rank : INDEX_NONE;
}

// ---------------------------------------------------------------------------------------------------------------------
int32 UEGT_ScoreboardSubsystem::SubmitScore(FName LevelKey, const FText& LevelDisplayName, const FString& PlayerName, int32 Score, int32 MaxEntries)
{
	const int32 Rank = GetRankForScore(LevelKey, Score, MaxEntries);
	if (Rank == INDEX_NONE)
	{
		return INDEX_NONE;
	}

	FEGT_ScoreboardEntry Entry;
	Entry.PlayerName = PlayerName.TrimStartAndEnd().Left(MaxPlayerNameLength);
	if (Entry.PlayerName.IsEmpty())
	{
		Entry.PlayerName = TEXT("Anonymous");
	}
	Entry.Score = Score;
	Entry.LevelDisplayName = LevelDisplayName;
	Entry.Date = FDateTime::Now();

	TArray<FEGT_ScoreboardEntry>& Entries = SaveGame->Boards.FindOrAdd(LevelKey).Entries;
	Entries.Insert(Entry, Rank);
	if (Entries.Num() > MaxEntries)
	{
		Entries.SetNum(MaxEntries);
	}

	// Tiny file: a synchronous save is fine
	if (!UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0))
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[Scoreboard] Failed to save slot '%s'"), *SaveSlotName);
	}

	UE_LOG(LogEmbersGameplayTest, Log, TEXT("[Scoreboard] '%s' scored %d on '%s': rank %d"), *Entry.PlayerName, Score, *LevelKey.ToString(), Rank + 1);
	return Rank;
}
