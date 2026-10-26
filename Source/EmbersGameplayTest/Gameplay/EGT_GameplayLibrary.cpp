// Elise Rosati


#include "Gameplay/EGT_GameplayLibrary.h"

#include "EmbersGameplayTest.h"
#include "EmbersGameplayTestGameMode.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

// ---------------------------------------------------------------------------------------------------------------------
FLinearColor UEGT_GameplayLibrary::GetStickerLinearColor(const UObject* WorldContextObject, EEGT_StickerColor StickerColor)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (const AEmbersGameplayTestGameMode* GameMode = World ? World->GetAuthGameMode<AEmbersGameplayTestGameMode>() : nullptr)
	{
		return GameMode->GetStickerLinearColor(StickerColor);
	}

	UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[GameplayLibrary] No valid gamemode found to resolve sticker colour '%s'"), *UEnum::GetValueAsString(StickerColor));
	return FLinearColor(1.0f, 0.0f, 1.0f);
}
