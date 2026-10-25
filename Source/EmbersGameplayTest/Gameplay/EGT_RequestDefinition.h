// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Gameplay/EGT_RequestTypes.h"
#include "EGT_RequestDefinition.generated.h"

/**
 * Description sheet of a reusable request (required stickers, points, max time)
 */
UCLASS(BlueprintType)
class EMBERSGAMEPLAYTEST_API UEGT_RequestDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// PROPERTIES
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Request", meta = (ShowOnlyInnerProperties))
	FEGT_Request Request;
};
