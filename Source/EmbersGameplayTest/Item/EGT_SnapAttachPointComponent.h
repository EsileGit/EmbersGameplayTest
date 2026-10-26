// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "EGT_SnapAttachPointComponent.generated.h"

/**
 * Marks the point of a parcel that lands on a snap spot's arrow.
 * The parcel is rotated so that this arrow's orientation matches the snap spot's one (the pointy arrow must "go into" the parcel).
 */
UCLASS(ClassGroup=(Parcel), meta=(BlueprintSpawnableComponent))
class EMBERSGAMEPLAYTEST_API UEGT_SnapAttachPointComponent : public UArrowComponent
{
	GENERATED_BODY()

public:
	UEGT_SnapAttachPointComponent();
};
