// Elise Rosati


#include "Item/EGT_DamageDecalComponent.h"

// ---------------------------------------------------------------------------------------------------------------------
UEGT_DamageDecalComponent::UEGT_DamageDecalComponent()
{
	DecalSize = FVector(2.0f, 10.0f, 10.0f);

	SetHiddenInGame(true);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_DamageDecalComponent::RefreshForDamageState(EEGT_ParcelDamageState DamageState)
{
	SetHiddenInGame(DamageState < VisibleFromState);
}
