// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EGT_ParcelDefinition.generated.h"

class AEGT_ParcelBase;

/**
 * Description sheet of a parcel type:
 * the parcel BP to spawn
 * the range its weight is drafted from
 */
UCLASS(BlueprintType)
class EMBERSGAMEPLAYTEST_API UEGT_ParcelDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Drafts a mass within MassRangeKg */
	float RollMassKg() const;

	// PROPERTIES
	// Name shown by the inspect UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel")
	FText DisplayName;

	// BP spawned for this type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel")
	TSubclassOf<AEGT_ParcelBase> ParcelClass;

	// Range the parcel's mass is drafted from at spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel", meta = (Units = "kg"))
	FFloatInterval MassRangeKg = FFloatInterval(1.0f, 200.0f);

};
