// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Components/DecalComponent.h"
#include "Item/EGT_ParcelDamageTypes.h"
#include "EGT_DamageDecalComponent.generated.h"

/**
 * Damage mark on a parcel, hidden in game until the parcel reaches VisibleFromState (or worse).
 */
UCLASS(ClassGroup=(Parcel), meta=(BlueprintSpawnableComponent))
class EMBERSGAMEPLAYTEST_API UEGT_DamageDecalComponent : public UDecalComponent
{
	GENERATED_BODY()

public:
	UEGT_DamageDecalComponent();

	// Shows the decal if DamageState is VisibleFromState or worse
	void RefreshForDamageState(EEGT_ParcelDamageState DamageState);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	EEGT_ParcelDamageState VisibleFromState = EEGT_ParcelDamageState::Dented;
};
