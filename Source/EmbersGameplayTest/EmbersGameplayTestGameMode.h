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

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AEmbersGameplayTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEmbersGameplayTestGameMode();

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

	UFUNCTION(BlueprintPure, Category="Requests")
	int32 GetScore() const { return Score; }

	// Fired every time the score changes
	UPROPERTY(BlueprintAssignable, Category="Requests")
	FOnScoreChanged OnScoreChanged;

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
};



