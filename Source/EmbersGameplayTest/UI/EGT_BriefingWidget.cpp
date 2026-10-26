// Elise Rosati


#include "UI/EGT_BriefingWidget.h"

#include "EmbersGameplayTestGameMode.h"

#include "Engine/World.h"

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_BriefingWidget::StartLevel()
{
	if (AEmbersGameplayTestGameMode* GameMode = GetWorld()->GetAuthGameMode<AEmbersGameplayTestGameMode>())
	{
		GameMode->StartLevel();
	}
}
