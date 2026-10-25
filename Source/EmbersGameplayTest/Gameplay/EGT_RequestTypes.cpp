// Elise Rosati


#include "Gameplay/EGT_RequestTypes.h"

#include "EmbersGameplayTest.h"
#include "Item/EGT_ParcelDefinition.h"

// ---------------------------------------------------------------------------------------------------------------------
int32 FEGT_Request::GetPointsAt(double Timestamp) const
{
	if (!HasStarted())
	{
		return MaxPoints;
	}

	const double ElapsedTime = FMath::Max(0.0, Timestamp - StartTimestamp);
	const double DecayAlpha = MaxTime > 0.0f ? FMath::Min(ElapsedTime / MaxTime, 1.0) : 1.0;
	return FMath::Lerp(MaxPoints, MinPoints, DecayAlpha);
}

// ---------------------------------------------------------------------------------------------------------------------
bool FEGT_Request::MatchesStickers(const TArray<EEGT_StickerColor>& Stickers) const
{
	if (Stickers.Num() != RequiredStickers.Num())
	{
		return false;
	}

	// Sorted copies: order doesn't matter, but duplicated colors do
	TArray<EEGT_StickerColor> SortedRequired = RequiredStickers;
	TArray<EEGT_StickerColor> SortedStickers = Stickers;
	SortedRequired.Sort();
	SortedStickers.Sort();
	return SortedRequired == SortedStickers;
}

// ---------------------------------------------------------------------------------------------------------------------
void FEGT_Request::MakeRequestFromDefinition()
{
	if (!ParcelDefinition || ParcelDefinition->StickerColors.IsEmpty())
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[Request] Can't draft a request: parcel definition '%s' is missing or has no colors"),
			*GetNameSafe(ParcelDefinition));
		return;
	}

	// Drafts a number of stickers between min and max and among definition possible colors
	const int32 StickerCount = ParcelDefinition->RollStickerCount();
	RequiredStickers.Reset(StickerCount);
	for (int32 StickerIndex = 0; StickerIndex < StickerCount; ++StickerIndex)
	{
		RequiredStickers.Add(ParcelDefinition->RollStickerColor());
	}
}
