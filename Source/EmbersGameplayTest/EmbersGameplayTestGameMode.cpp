// Copyright Epic Games, Inc. All Rights Reserved.

#include "EmbersGameplayTestGameMode.h"

#include "EmbersGameplayTest.h"

#include "Gameplay/EGT_RequestDefinition.h"

AEmbersGameplayTestGameMode::AEmbersGameplayTestGameMode()
{
	StickerColors.Add(EEGT_StickerColor::Red, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Blue, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Green, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Yellow, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Purple, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Orange, FLinearColor());
}

// ---------------------------------------------------------------------------------------------------------------------
void AEmbersGameplayTestGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// Beware of a potential race condition here: at the moment, no parcels are created at level begin (contrary to Cat Mail and Co.)
	// so it's ok that the init of requests is done in a begin play. If anotheractor were to neeed access to the requests
	// during its own begin play, this code shall be moved somewhere earlier
	CreateRandomRequests();
}

// ---------------------------------------------------------------------------------------------------------------------
void AEmbersGameplayTestGameMode::CreateRandomRequests()
{
	Requests.Reset(RequestDefinitions.Num());
	for (const UEGT_RequestDefinition* Definition : RequestDefinitions)
	{
		if (!Definition)
		{
			continue;
		}

		FEGT_Request& Request = Requests.Add_GetRef(Definition->Request);
		if (!Request.bIsCustomRequest)
		{
			Request.MakeRequestFromDefinition();
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------
FLinearColor AEmbersGameplayTestGameMode::GetStickerLinearColor(EEGT_StickerColor StickerColor) const
{
	if (const FLinearColor* LinearColor = StickerColors.Find(StickerColor))
	{
		return *LinearColor;
	}

	UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[GameMode] No colour mapped for sticker colour '%s'"), *UEnum::GetValueAsString(StickerColor));
	return FLinearColor(1.0f, 0.0f, 1.0f);
}

#pragma region Requests

// ---------------------------------------------------------------------------------------------------------------------
bool AEmbersGameplayTestGameMode::StartRequest(int32 RequestIndex)
{
	if (!Requests.IsValidIndex(RequestIndex))
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[GameMode] Can't start request %d: invalid index (%d requests)"), RequestIndex, Requests.Num());
		return false;
	}

	Requests[RequestIndex].StartTimestamp = GetWorld()->GetTimeSeconds();
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------
int32 AEmbersGameplayTestGameMode::GetRequestCurrentPoints(const FEGT_Request& Request) const
{
	return Request.GetPointsAt(GetWorld()->GetTimeSeconds());
}

// ---------------------------------------------------------------------------------------------------------------------
const FEGT_Request* AEmbersGameplayTestGameMode::GetRequest(int32 RequestIndex) const
{
	return Requests.IsValidIndex(RequestIndex) ? &Requests[RequestIndex] : nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------
int32 AEmbersGameplayTestGameMode::DraftRequest()
{
	TArray<int32> AvailableIndices;
	for (int32 RequestIndex = 0; RequestIndex < Requests.Num(); ++RequestIndex)
	{
		if (Requests[RequestIndex].Status == EEGT_RequestStatus::Available)
		{
			AvailableIndices.Add(RequestIndex);
		}
	}

	if (AvailableIndices.IsEmpty())
	{
		return INDEX_NONE;
	}

	const int32 RequestIndex = AvailableIndices[FMath::RandRange(0, AvailableIndices.Num() - 1)];
	Requests[RequestIndex].Status = EEGT_RequestStatus::InProgress;
	StartRequest(RequestIndex);
	return RequestIndex;
}

// ---------------------------------------------------------------------------------------------------------------------
int32 AEmbersGameplayTestGameMode::ClaimRequestForParcel(int32 MaxStickerCount)
{
	TArray<int32> EligibleIndices;
	for (int32 RequestIndex = 0; RequestIndex < Requests.Num(); ++RequestIndex)
	{
		const FEGT_Request& CurrentRequest = Requests[RequestIndex];
		const int32 StickerCount = CurrentRequest.RequiredStickers.Num();
		const bool bCanStillBeDone = CurrentRequest.Status == EEGT_RequestStatus::InProgress || CurrentRequest.Status == EEGT_RequestStatus::Available;
		if (bCanStillBeDone && 
			!CurrentRequest.bIsDoable && // We mark the requests so that different spawn with policy FromLevelRequirement don't overlap
			StickerCount > 0 && 
			StickerCount <= MaxStickerCount)
		{
			EligibleIndices.Add(RequestIndex);
		}
	}

	if (EligibleIndices.IsEmpty())
	{
		return INDEX_NONE;
	}

	const int32 RequestIndex = EligibleIndices[FMath::RandRange(0, EligibleIndices.Num() - 1)];
	Requests[RequestIndex].bIsDoable = true;
	return RequestIndex;
}

// ---------------------------------------------------------------------------------------------------------------------
void AEmbersGameplayTestGameMode::ReleaseRequestForParcel(int32 RequestIndex)
{
	if (Requests.IsValidIndex(RequestIndex))
	{
		Requests[RequestIndex].bIsDoable = false;
	}
}

// ---------------------------------------------------------------------------------------------------------------------
int32 AEmbersGameplayTestGameMode::CompleteRequest(int32 RequestIndex, bool bSuccess)
{
	if (!Requests.IsValidIndex(RequestIndex) || Requests[RequestIndex].Status != EEGT_RequestStatus::InProgress)
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[GameMode] Can't complete request %d: invalid index or not in progress"), RequestIndex);
		return 0;
	}

	FEGT_Request& Request = Requests[RequestIndex];
	Request.Status = bSuccess ? EEGT_RequestStatus::Succeeded : EEGT_RequestStatus::Failed;
	Request.AwardedPoints = bSuccess ? GetRequestCurrentPoints(Request) : 0;

	if (Request.AwardedPoints != 0)
	{
		Score += Request.AwardedPoints;
		OnScoreChanged.Broadcast(Score);
	}

	UE_LOG(LogEmbersGameplayTest, Log, TEXT("[GameMode] Request %d %s: +%d pts (score: %d)"), RequestIndex,
		bSuccess ? TEXT("succeeded") : TEXT("failed"), Request.AwardedPoints, Score);
	return Request.AwardedPoints;
}

#pragma endregion
