// Elise Rosati


#include "Item/EGT_SnapAttachPointComponent.h"

// ---------------------------------------------------------------------------------------------------------------------
UEGT_SnapAttachPointComponent::UEGT_SnapAttachPointComponent()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set arrow default size
	ArrowLength = 40.0f;
	ArrowSize = 0.3f;
	ArrowColor = FColor::Cyan;

	SetHiddenInGame(true);
}
