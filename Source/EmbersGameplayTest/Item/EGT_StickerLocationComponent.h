// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "EGT_StickerLocationComponent.generated.h"

/**
 * Marks a spot on a parcel where a sticker can be spawned.
 * The arrow is the decal's projection direction (it needs to face the... face of the parcel. Or face it "outwards").
 * If somebody is colorblind, I can add the name of the color
 */
UCLASS(ClassGroup=(Parcel), meta=(BlueprintSpawnableComponent))
class EMBERSGAMEPLAYTEST_API UEGT_StickerLocationComponent : public UArrowComponent
{
	GENERATED_BODY()

public:
	UEGT_StickerLocationComponent();
};
