// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Item/EGT_StickerTypes.h"
#include "EGT_GameplayLibrary.generated.h"

/**
 * Gameplay helpers usable from anywhere (C++ or Blueprint) that has a world context
 */
UCLASS()
class EMBERSGAMEPLAYTEST_API UEGT_GameplayLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Actual colour of a named sticker colour, as mapped on the game mode (magenta if unmapped or no game mode)
	UFUNCTION(BlueprintPure, Category="Embers|Stickers", meta = (WorldContext = "WorldContextObject"))
	static FLinearColor GetStickerLinearColor(const UObject* WorldContextObject, EEGT_StickerColor StickerColor);
};
