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
