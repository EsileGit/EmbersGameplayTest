// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Gameplay/EGT_RequestTypes.h"
#include "Gameplay/EGT_ScoreboardSaveGame.h"
#include "Item/EGT_ParcelDamageTypes.h"
#include "Item/EGT_StickerTypes.h"
#include "EmbersGameplayTestGameMode.generated.h"

class UEGT_RequestDefinition;

/** Broadcast every time the score changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);

/** Flow of a game level: rules screen, timed play, results screen */
UENUM(BlueprintType)
enum class EEGT_LevelPhase : uint8
{
	Briefing,
	Playing,
	Ended
};

/** Broadcast every time the level phase changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelPhaseChanged, EEGT_LevelPhase, NewPhase);

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AEmbersGameplayTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEmbersGameplayTestGameMode();
	virtual void StartPlay() override;

	/** Actual colour of a named sticker colour (magenta if unmapped) */
	UFUNCTION(BlueprintPure, Category="Stickers")
	FLinearColor GetStickerLinearColor(EEGT_StickerColor StickerColor) const;

#pragma region Requests

	UFUNCTION(BlueprintPure, Category="Requests")
	const TArray<FEGT_Request>& GetRequests() const { return Requests; }

	UFUNCTION(BlueprintCallable, Category="Requests")
	bool StartRequest(int32 RequestIndex);

	// Points the request is worth right now
	UFUNCTION(BlueprintPure, Category="Requests")
	int32 GetRequestCurrentPoints(const FEGT_Request& Request) const;

	// Null if the index is invalid
	const FEGT_Request* GetRequest(int32 RequestIndex) const;

	// Picks a random Available request and starts it. Returns its index, or INDEX_NONE if none is left
	UFUNCTION(BlueprintCallable, Category="Requests")
	int32 DraftRequest();

	// Closes an InProgress request. On success, it grants its current points, scaled by the delivered parcel's damage state.
	// Returns the points granted
	UFUNCTION(BlueprintCallable, Category="Requests")
	int32 CompleteRequest(int32 RequestIndex, bool bSuccess, EEGT_ParcelDamageState DamageState = EEGT_ParcelDamageState::Intact);

	// Picks a random Available, non-doable request with 1 to MaxStickerCount stickers and marks it doable.
	// Returns its index, or INDEX_NONE if none fits
	int32 ClaimRequestForParcel(int32 MaxStickerCount);

	// The parcel made for this request is gone: another parcel can be made for it (if it's still available). 
	// This is to prevent cases where a Parcel meant for Request A is used (successfully) for Request B, making Request A
	// not guaranted doable.
	void ReleaseRequestForParcel(int32 RequestIndex);

	UFUNCTION(BlueprintPure, Category="Requests")
	int32 GetScore() const { return Score; }

	// Fired every time the score changes
	UPROPERTY(BlueprintAssignable, Category="Requests")
	FOnScoreChanged OnScoreChanged;

#pragma endregion

#pragma region Damage

	// Damage state of a parcel with this integrity (0-1)
	UFUNCTION(BlueprintPure, Category="Parcel|Damage")
	EEGT_ParcelDamageState GetDamageStateForIntegrity(float IntegrityRatio) const;

#pragma endregion

#pragma region LevelFlow

	// Leaves the briefing: unfreezes the player and starts the level timer
	UFUNCTION(BlueprintCallable, Category="Level Flow")
	void StartLevel();

	// Reloads the current level from scratch
	UFUNCTION(BlueprintCallable, Category="Level Flow")
	void RestartLevel();

	UFUNCTION(BlueprintPure, Category="Level Flow")
	EEGT_LevelPhase GetLevelPhase() const { return LevelPhase; }

	UFUNCTION(BlueprintPure, Category="Level Flow")
	bool IsLevelPlaying() const { return LevelPhase == EEGT_LevelPhase::Playing; }

	// Time left on the level timer (full duration during the briefing, 0 once ended or if not a game level)
	UFUNCTION(BlueprintPure, Category="Level Flow")
	float GetLevelRemainingTime() const;

	// Fired every time the level phase changes
	UPROPERTY(BlueprintAssignable, Category="Level Flow")
	FOnLevelPhaseChanged OnLevelPhaseChanged;

	// Player-facing name of the level (falls back to the level asset name if LevelDisplayName is empty)
	UFUNCTION(BlueprintPure, Category="Level Flow")
	FText GetLevelDisplayName() const;

#pragma endregion

#pragma region Scoreboard

	// Rank (0 = best) the current score would take on this level's scoreboard, or INDEX_NONE if it doesn't make it
	UFUNCTION(BlueprintPure, Category="Level Flow|Scoreboard")
	int32 GetScoreboardRank() const;

	// Records the current score on this level's scoreboard, once the level has ended (only once per run).
	// Returns its rank, or INDEX_NONE if not recorded
	UFUNCTION(BlueprintCallable, Category="Level Flow|Scoreboard")
	int32 SubmitScore(const FString& PlayerName);

	// This level's scoreboard, best first
	UFUNCTION(BlueprintPure, Category="Level Flow|Scoreboard")
	TArray<FEGT_ScoreboardEntry> GetScoreboardEntries() const;

#pragma endregion

protected:
	virtual void BeginPlay() override;
	void CreateRandomRequests();

	// Map: sticker color enum -> actual color
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stickers")
	TMap<EEGT_StickerColor, FLinearColor> StickerColors;

#pragma region Requests

	// The level's requests, copied into Requests at BeginPlay
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Requests")
	TArray<TObjectPtr<UEGT_RequestDefinition>> RequestDefinitions;

	// Runtime
	// Rolled copies of the requests definitions
	UPROPERTY(Transient, VisibleAnywhere)
	TArray<FEGT_Request> Requests;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Requests")
	int32 Score = 0;

#pragma endregion

#pragma region Damage

	// A parcel is in a damage state when its integrity is at or below that state's threshold (the worst state wins)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parcel|Damage", meta = (ClampMin = 0, ClampMax = 100, Units = "Percent"))
	float DentedThresholdPercent = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parcel|Damage", meta = (ClampMin = 0, ClampMax = 100, Units = "Percent"))
	float DamagedThresholdPercent = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parcel|Damage", meta = (ClampMin = 0, ClampMax = 100, Units = "Percent"))
	float BrokenThresholdPercent = 0.0f;

	// Map: damage state of the parcel -> coefficient applied to the request's points (negative allowed => penalty)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Requests|Points")
	TMap<EEGT_ParcelDamageState, float> DamagePointsMultipliers;

#pragma endregion

#pragma region LevelFlow

	// Freezes the player outside of the Playing phase and notifies listeners
	void SetLevelPhase(EEGT_LevelPhase NewPhase);

	// Level timer callback
	void EndLevel();

	// PROPERTIES
	// Real game level: rules screen, timed play and results screen. Untick for test scenes (free play, no timer)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Level Flow")
	bool bIsGameLevel = true;

	// Duration of the timed play
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Level Flow", meta = (EditCondition = "bIsGameLevel", ClampMin = 1, Units = "s"))
	float LevelDuration = 180.0f;

	// Player-facing name of the level. Display only: scoreboards are keyed by the level asset name, so renaming this keeps them.
	// Levels sharing this game mode BP share this name too (make a child BP if they need different ones)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Level Flow")
	FText LevelDisplayName;

	// Runtime
	UPROPERTY(Transient, VisibleAnywhere, Category="Level Flow")
	EEGT_LevelPhase LevelPhase = EEGT_LevelPhase::Playing;

	FTimerHandle LevelTimerHandle;

#pragma endregion

#pragma region Scoreboard

	// Key of this level in the saved scoreboards (level asset name, PIE prefix stripped)
	FName GetScoreboardLevelKey() const;

	// PROPERTIES
	// Number of best runs kept on this level's scoreboard
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Level Flow|Scoreboard", meta = (EditCondition = "bIsGameLevel", ClampMin = 1))
	int32 ScoreboardSize = 5;

	// Runtime
	UPROPERTY(Transient, VisibleAnywhere, Category="Level Flow|Scoreboard")
	bool bScoreSubmitted = false;

#pragma endregion
};



