// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Item/EGT_StickerTypes.h"
#include "EGT_ParcelDefinition.generated.h"

class AEGT_ParcelBase;

/**
 * How a parcel's stickers are chosen at spawn
 */
UENUM(BlueprintType)
enum class EEGT_StickerSpawnPolicy : uint8
{
	// Drafted among StickerColors
	Random,
	// Copied from a level request
	FromLevelRequirement,
	// Random or FromLevelRequirement
	Alternate
};

/**
 * Description sheet of a parcel type:
 * the parcel BP to spawn
 * the range its weight is drafted from
 * its stickers colours
 */
UCLASS(BlueprintType)
class EMBERSGAMEPLAYTEST_API UEGT_ParcelDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Drafts a mass within MassRangeKg */
	float RollMassKg() const;

	/** Drafts a sticker count within [MinStickerCount, MaxStickerCount] */
	int32 RollStickerCount() const;

	/** Drafts a color among StickerColors */
	EEGT_StickerColor RollStickerColor() const;

	/** Drafts a sticker count, then a color for each sticker */
	TArray<EEGT_StickerColor> RollStickerColors() const;

	/** Whether this spawn should copy a level request's stickers, according to StickerSpawnPolicy */
	bool RollUsesLevelRequirement() const;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel|Stickers")
	EEGT_StickerSpawnPolicy StickerSpawnPolicy = EEGT_StickerSpawnPolicy::Random;

	// Probability that a spawn uses FromLevelRequirement instead of Random
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel|Stickers", meta = (ClampMin = 0, ClampMax = 1,
		EditCondition = "StickerSpawnPolicy == EEGT_StickerSpawnPolicy::Alternate", EditConditionHides))
	float LevelRequirementChance = 0.5f;

	// Colors the parcel's stickers are drafted from
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel|Stickers")
	TArray<EEGT_StickerColor> StickerColors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel|Stickers", meta = (ClampMin = 0))
	int32 MinStickerCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel|Stickers", meta = (ClampMin = 0))
	int32 MaxStickerCount = 1;

};
