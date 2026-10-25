// Elise Rosati


#include "Item/EGT_ParcelDefinition.h"

// ---------------------------------------------------------------------------------------------------------------------
float UEGT_ParcelDefinition::RollMassKg() const
{
	return FMath::FRandRange(MassRangeKg.Min, MassRangeKg.Max);
}

// ---------------------------------------------------------------------------------------------------------------------
int32 UEGT_ParcelDefinition::RollStickerCount() const
{
	// Safeguard towards a Min set above Max
	return FMath::RandRange(FMath::Min(MinStickerCount, MaxStickerCount), FMath::Max(MinStickerCount, MaxStickerCount));
}

// ---------------------------------------------------------------------------------------------------------------------
EEGT_StickerColor UEGT_ParcelDefinition::RollStickerColor() const
{
	check(!StickerColors.IsEmpty());
	return StickerColors[FMath::RandRange(0, StickerColors.Num() - 1)];
}
