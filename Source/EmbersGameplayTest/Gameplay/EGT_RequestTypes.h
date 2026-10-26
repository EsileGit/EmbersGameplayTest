// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "Item/EGT_StickerTypes.h"
#include "EGT_RequestTypes.generated.h"

class UEGT_ParcelDefinition;
class UEGT_RequestDefinition;

/**
 * Lifecycle of a request. Only Available requests can be drafted by a desk.
 */
UENUM(BlueprintType)
enum class EEGT_RequestStatus : uint8
{
	Available,
	InProgress,
	Succeeded,
	Failed
};

/**
 * A customer request: the stickers the wanted parcel must carry and the points it's worth.
 * MaxPoints: if the player delivers the request quickly (instant)
 * MinPoints: points for delivering after MaxTime
 */
USTRUCT(BlueprintType)
struct FEGT_Request
{
	GENERATED_BODY()

	bool HasStarted() const { return StartTimestamp >= 0.0; }

	// Points this request is worth at Timestamp, going linearly from MaxPoints to MinPoints over MaxTime
	int32 GetPointsAt(double Timestamp) const;

	// Time left at Timestamp before the request is only worth MinPoints (MaxTime if not started)
	float GetRemainingTimeAt(double Timestamp) const;

	// True if Stickers holds exactly the required colors, in any order
	bool MatchesStickers(const TArray<EEGT_StickerColor>& Stickers) const;

	void MakeRequestFromDefinition();

	// PROPERTIES
	// A custom request is set by hand. Otherwise, a random combination is drafted from a parcel definition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Request")
	bool bIsCustomRequest = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Request", meta = (EditCondition = "!bIsCustomRequest", EditConditionHides))
	TObjectPtr<UEGT_ParcelDefinition> ParcelDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Request", meta = (EditCondition = "bIsCustomRequest", EditConditionHides))
	TArray<EEGT_StickerColor> RequiredStickers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Request|Points", meta = (ClampMin = 0))
	int32 MaxPoints = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Request|Points", meta = (ClampMin = 0))
	int32 MinPoints = 20;

	// Time after which the request is only worth MinPoints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Request|Points", meta = (ClampMin = 0, Units = "s"))
	float MaxTime = 60.0f;

	// Runtime
	UPROPERTY(Transient, BlueprintReadOnly, Category="Request")
	double StartTimestamp = -1.0;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Request")
	EEGT_RequestStatus Status = EEGT_RequestStatus::Available;

	// A parcel was spawned with this request's stickers (FromLevelRequirement policy), so no other parcel claims it
	UPROPERTY(Transient, BlueprintReadOnly, Category="Request")
	bool bIsDoable = false;

	// Points granted when the request was completed (0 if failed)
	UPROPERTY(Transient, BlueprintReadOnly, Category="Request")
	int32 AwardedPoints = 0;
};