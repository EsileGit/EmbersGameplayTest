// Elise Rosati


#include "Item/EGT_StickerLocationComponent.h"

// ---------------------------------------------------------------------------------------------------------------------
UEGT_StickerLocationComponent::UEGT_StickerLocationComponent()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set arrow default size
	ArrowLength = 40.0f;
	ArrowSize = 0.3f;
	
	SetHiddenInGame(true);
}
