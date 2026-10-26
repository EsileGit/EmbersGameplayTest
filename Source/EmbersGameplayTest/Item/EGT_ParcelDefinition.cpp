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

// ---------------------------------------------------------------------------------------------------------------------
TArray<EEGT_StickerColor> UEGT_ParcelDefinition::RollStickerColors() const
{
	TArray<EEGT_StickerColor> Colors;
	if (StickerColors.IsEmpty())
	{
		return Colors;
	}

	const int32 StickerCount = RollStickerCount();
	Colors.Reserve(StickerCount);
	for (int32 StickerIndex = 0; StickerIndex < StickerCount; ++StickerIndex)
	{
		Colors.Add(RollStickerColor());
	}
	return Colors;
}

// ---------------------------------------------------------------------------------------------------------------------
bool UEGT_ParcelDefinition::RollUsesLevelRequirement() const
{
	switch (StickerSpawnPolicy)
	{
	case EEGT_StickerSpawnPolicy::FromLevelRequirement:
		return true;
	case EEGT_StickerSpawnPolicy::Alternate:
		return FMath::FRand() < LevelRequirementChance;
	default:
		return false;
	}
}
