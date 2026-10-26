// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "EGT_ParcelDamageTypes.generated.h"

/**
 * Damage tiers of a parcel, deduced from its integrity
 */
UENUM(BlueprintType)
enum class EEGT_ParcelDamageState : uint8
{
	Intact,
	Dented,
	Damaged,
	Broken
};
