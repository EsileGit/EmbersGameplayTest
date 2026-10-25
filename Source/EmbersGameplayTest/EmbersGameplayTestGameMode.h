// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Item/EGT_StickerTypes.h"
#include "EmbersGameplayTestGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AEmbersGameplayTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEmbersGameplayTestGameMode();

	/** Actual colour of a named sticker colour (magenta if unmapped) */
	UFUNCTION(BlueprintPure, Category="Stickers")
	FLinearColor GetStickerLinearColor(EEGT_StickerColor StickerColor) const;

protected:
	// Colour displayed for each named sticker colour
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stickers")
	TMap<EEGT_StickerColor, FLinearColor> StickerColors;
};



