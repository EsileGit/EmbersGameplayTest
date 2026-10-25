// Copyright Epic Games, Inc. All Rights Reserved.

#include "EmbersGameplayTestGameMode.h"

#include "EmbersGameplayTest.h"

AEmbersGameplayTestGameMode::AEmbersGameplayTestGameMode()
{
	StickerColors.Add(EEGT_StickerColor::Red, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Blue, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Green, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Yellow, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Purple, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Orange, FLinearColor());
}

FLinearColor AEmbersGameplayTestGameMode::GetStickerLinearColor(EEGT_StickerColor StickerColor) const
{
	if (const FLinearColor* LinearColor = StickerColors.Find(StickerColor))
	{
		return *LinearColor;
	}

	UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[GameMode] No colour mapped for sticker colour '%s'"), *UEnum::GetValueAsString(StickerColor));
	return FLinearColor(1.0f, 0.0f, 1.0f);
}
