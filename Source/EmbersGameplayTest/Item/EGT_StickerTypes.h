// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "EGT_StickerTypes.generated.h"

/**
 * Named sticker colors, used to identify parcels (for lack of a more complex system)
 */
UENUM(BlueprintType)
enum class EEGT_StickerColor : uint8
{
	Red,
	Blue,
	Green,
	Yellow,
	Purple,
	Orange
};
