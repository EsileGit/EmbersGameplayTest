// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/EGT_StickerTypes.h"
#include "EGT_Sticker.generated.h"

class UDecalComponent;

/**
 * Parcel sticker. For now a simple colored decal (no change of shape or anything else).
 */
UCLASS(Abstract, Blueprintable)
class EMBERSGAMEPLAYTEST_API AEGT_Sticker : public AActor
{
	GENERATED_BODY()

public:
	AEGT_Sticker();

	UFUNCTION(BlueprintCallable, Category="Sticker")
	void SetStickerColor(EEGT_StickerColor InColor);

	UFUNCTION(BlueprintPure, Category="Sticker")
	EEGT_StickerColor GetStickerColor() const { return StickerColor; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDecalComponent> StickerDecal;

	// Name of the vector parameter driving the colour in the decal material
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sticker")
	FName ColorParameterName = TEXT("Color");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Sticker")
	EEGT_StickerColor StickerColor = EEGT_StickerColor::Red;

	// Blueprint reactions hook (for cosmetic)
	UFUNCTION(BlueprintImplementableEvent, Category="Sticker")
	void OnStickerColorSet(EEGT_StickerColor InColor, FLinearColor LinearColor);

};
