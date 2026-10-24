// Elise Rosati


#include "Item/EGT_ParcelDefinition.h"

// ---------------------------------------------------------------------------------------------------------------------
float UEGT_ParcelDefinition::RollMassKg() const
{
	return FMath::FRandRange(MassRangeKg.Min, MassRangeKg.Max);
}
