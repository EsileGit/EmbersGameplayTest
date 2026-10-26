// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Gameplay/EGT_RequestTypes.h"
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

	// Closes an InProgress request. On success, it grants its current points. Returns the points granted
	UFUNCTION(BlueprintCallable, Category="Requests")
	int32 CompleteRequest(int32 RequestIndex, bool bSuccess);

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

	// Runtime
	UPROPERTY(Transient, VisibleAnywhere, Category="Level Flow")
	EEGT_LevelPhase LevelPhase = EEGT_LevelPhase::Playing;

	FTimerHandle LevelTimerHandle;

#pragma endregion
};



