// Copyright Epic Games, Inc. All Rights Reserved.

#include "EmbersGameplayTestGameMode.h"

#include "EmbersGameplayTest.h"
#include "EmbersGameplayTestPlayerController.h"

#include "Gameplay/EGT_RequestDefinition.h"
#include "Gameplay/EGT_ScoreboardSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AEmbersGameplayTestGameMode::AEmbersGameplayTestGameMode()
{
	StickerColors.Add(EEGT_StickerColor::Red, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Blue, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Green, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Yellow, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Purple, FLinearColor());
	StickerColors.Add(EEGT_StickerColor::Orange, FLinearColor());

	DamagePointsMultipliers.Add(EEGT_ParcelDamageState::Intact, 1.0f);
	DamagePointsMultipliers.Add(EEGT_ParcelDamageState::Dented, 0.75f);
	DamagePointsMultipliers.Add(EEGT_ParcelDamageState::Damaged, 0.4f);
	DamagePointsMultipliers.Add(EEGT_ParcelDamageState::Broken, 0.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEmbersGameplayTestGameMode::StartPlay()
{
	// Before other begin plays so that other actor created at the start see the correct phase (this avoids the potential race
	// condition described just below in the BeginPlay).
	SetLevelPhase(bIsGameLevel ? EEGT_LevelPhase::Briefing : EEGT_LevelPhase::Playing);

	Super::StartPlay();
}

// ---------------------------------------------------------------------------------------------------------------------
void AEmbersGameplayTestGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// Beware of a potential race condition here: at the moment, no parcels are created at level begin (contrary to Cat Mail and Co.)
	// so it's ok that the init of requests is done in a begin play. If another actor were to need access to the requests
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
int32 AEmbersGameplayTestGameMode::CompleteRequest(int32 RequestIndex, bool bSuccess, EEGT_ParcelDamageState DamageState)
{
	if (!Requests.IsValidIndex(RequestIndex) || Requests[RequestIndex].Status != EEGT_RequestStatus::InProgress)
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[GameMode] Can't complete request %d: invalid index or not in progress"), RequestIndex);
		return 0;
	}

	FEGT_Request& Request = Requests[RequestIndex];
	Request.Status = bSuccess ? EEGT_RequestStatus::Succeeded : EEGT_RequestStatus::Failed;
	const float* FoundMultiplier = DamagePointsMultipliers.Find(DamageState);
	const float DamageMultiplier = FoundMultiplier ? *FoundMultiplier : 1.0f;
	Request.AwardedPoints = bSuccess ? FMath::RoundToInt(GetRequestCurrentPoints(Request) * DamageMultiplier) : 0;

	if (Request.AwardedPoints != 0)
	{
		Score += Request.AwardedPoints;
		OnScoreChanged.Broadcast(Score);
	}

	UE_LOG(LogEmbersGameplayTest, Log, TEXT("[GameMode] Request %d %s: %+d pts (damage: %s, x%.2f) (score: %d)"), RequestIndex,
		bSuccess ? TEXT("succeeded") : TEXT("failed"), Request.AwardedPoints, *UEnum::GetValueAsString(DamageState), DamageMultiplier, Score);
	return Request.AwardedPoints;
}

#pragma endregion

#pragma region Damage

// ---------------------------------------------------------------------------------------------------------------------
EEGT_ParcelDamageState AEmbersGameplayTestGameMode::GetDamageStateForIntegrity(float IntegrityRatio) const
{
	const float IntegrityPercent = IntegrityRatio * 100.0f;
	if (IntegrityPercent <= BrokenThresholdPercent)
	{
		return EEGT_ParcelDamageState::Broken;
	}
	if (IntegrityPercent <= DamagedThresholdPercent)
	{
		return EEGT_ParcelDamageState::Damaged;
	}
	if (IntegrityPercent <= DentedThresholdPercent)
	{
		return EEGT_ParcelDamageState::Dented;
	}
	return EEGT_ParcelDamageState::Intact;
}

#pragma endregion

#pragma region LevelFlow

// ---------------------------------------------------------------------------------------------------------------------
void AEmbersGameplayTestGameMode::SetLevelPhase(EEGT_LevelPhase NewPhase)
{
	LevelPhase = NewPhase;

	// Pure solo game: the first player controller is the player
	if (AEmbersGameplayTestPlayerController* PlayerController = GetWorld()->GetFirstPlayerController<AEmbersGameplayTestPlayerController>())
	{
		PlayerController->SetPlayerFrozen(NewPhase != EEGT_LevelPhase::Playing);
	}

	UE_LOG(LogEmbersGameplayTest, Log, TEXT("[GameMode] Level phase: %s"), *UEnum::GetValueAsString(NewPhase));
	OnLevelPhaseChanged.Broadcast(NewPhase);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEmbersGameplayTestGameMode::StartLevel()
{
	if (LevelPhase != EEGT_LevelPhase::Briefing)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(LevelTimerHandle, this, &ThisClass::EndLevel, LevelDuration, false);
	SetLevelPhase(EEGT_LevelPhase::Playing);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEmbersGameplayTestGameMode::EndLevel()
{
	SetLevelPhase(EEGT_LevelPhase::Ended);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEmbersGameplayTestGameMode::RestartLevel()
{
	UGameplayStatics::OpenLevel(this, FName(UGameplayStatics::GetCurrentLevelName(this)));
}

// ---------------------------------------------------------------------------------------------------------------------
float AEmbersGameplayTestGameMode::GetLevelRemainingTime() const
{
	if (LevelPhase == EEGT_LevelPhase::Briefing)
	{
		return LevelDuration;
	}

	// GetTimerRemaining returns -1 when the timer isn't running
	return FMath::Max(GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle), 0.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
FText AEmbersGameplayTestGameMode::GetLevelDisplayName() const
{
	return LevelDisplayName.IsEmpty() ? FText::FromString(UGameplayStatics::GetCurrentLevelName(this)) : LevelDisplayName;
}

#pragma endregion

#pragma region Scoreboard

// ---------------------------------------------------------------------------------------------------------------------
FName AEmbersGameplayTestGameMode::GetScoreboardLevelKey() const
{
	return FName(UGameplayStatics::GetCurrentLevelName(this));
}

// ---------------------------------------------------------------------------------------------------------------------
int32 AEmbersGameplayTestGameMode::GetScoreboardRank() const
{
	if (bScoreSubmitted)
	{
		return INDEX_NONE;
	}

	return GetGameInstance()->GetSubsystem<UEGT_ScoreboardSubsystem>()->GetRankForScore(GetScoreboardLevelKey(), Score, ScoreboardSize);
}

// ---------------------------------------------------------------------------------------------------------------------
int32 AEmbersGameplayTestGameMode::SubmitScore(const FString& PlayerName)
{
	if (LevelPhase != EEGT_LevelPhase::Ended || bScoreSubmitted)
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[GameMode] Can't submit score: level not ended or score already submitted"));
		return INDEX_NONE;
	}

	bScoreSubmitted = true;
	return GetGameInstance()->GetSubsystem<UEGT_ScoreboardSubsystem>()->SubmitScore(GetScoreboardLevelKey(), GetLevelDisplayName(), PlayerName, Score, ScoreboardSize);
}

// ---------------------------------------------------------------------------------------------------------------------
TArray<FEGT_ScoreboardEntry> AEmbersGameplayTestGameMode::GetScoreboardEntries() const
{
	return GetGameInstance()->GetSubsystem<UEGT_ScoreboardSubsystem>()->GetEntries(GetScoreboardLevelKey());
}

#pragma endregion
