// Elise Rosati


#include "UI/EGT_ResultsWidget.h"

#include "EmbersGameplayTestGameMode.h"

#include "Engine/World.h"

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ResultsWidget::RestartLevel()
{
	if (AEmbersGameplayTestGameMode* GameMode = GetWorld()->GetAuthGameMode<AEmbersGameplayTestGameMode>())
	{
		GameMode->RestartLevel();
	}
}

// ---------------------------------------------------------------------------------------------------------------------
FText UEGT_ResultsWidget::GetLevelDisplayName() const
{
	const AEmbersGameplayTestGameMode* GameMode = GetWorld()->GetAuthGameMode<AEmbersGameplayTestGameMode>();
	return GameMode ? GameMode->GetLevelDisplayName() : FText::GetEmpty();
}

// ---------------------------------------------------------------------------------------------------------------------
int32 UEGT_ResultsWidget::SubmitPlayerName(const FString& PlayerName)
{
	AEmbersGameplayTestGameMode* GameMode = GetWorld()->GetAuthGameMode<AEmbersGameplayTestGameMode>();
	return GameMode ? GameMode->SubmitScore(PlayerName) : INDEX_NONE;
}

// ---------------------------------------------------------------------------------------------------------------------
TArray<FEGT_ScoreboardEntry> UEGT_ResultsWidget::GetScoreboardEntries() const
{
	const AEmbersGameplayTestGameMode* GameMode = GetWorld()->GetAuthGameMode<AEmbersGameplayTestGameMode>();
	return GameMode ? GameMode->GetScoreboardEntries() : TArray<FEGT_ScoreboardEntry>();
}
