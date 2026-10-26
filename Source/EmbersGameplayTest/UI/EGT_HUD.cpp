// Elise Rosati


#include "UI/EGT_HUD.h"

#include "EmbersGameplayTest.h"
#include "EmbersGameplayTestGameMode.h"
#include "Gameplay/EGT_GameplayLibrary.h"
#include "Item/EGT_ParcelDefinition.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<bool> CVarDebugRequests(
	TEXT("EGT.Debug.Requests"),
	false,
	TEXT("Lists the game mode's requests and their state on screen"));
#endif

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_HUD::BeginPlay()
{
	Super::BeginPlay();

	// Standard boilerplate, in a solo-player game, we could remove it, but this is quite harmless
	if (!PlayerOwner || !PlayerOwner->IsLocalPlayerController())
	{
		return;
	}

	if (!OverlayWidgetClass)
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("'%s' has no HUDWidgetClass set: no HUD will be displayed."), *GetNameSafe(this));
		return;
	}

	OverlayWidget = CreateWidget<UUserWidget>(PlayerOwner, OverlayWidgetClass);
	if (OverlayWidget)
	{
		OverlayWidget->AddToPlayerScreen(0);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_HUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OverlayWidget)
	{
		OverlayWidget->RemoveFromParent();
		OverlayWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_HUD::DrawHUD()
{
	Super::DrawHUD();

#if !UE_BUILD_SHIPPING
	if (CVarDebugRequests.GetValueOnGameThread())
	{
		DrawRequestsDebug();
	}
#endif
}

#pragma region Debug

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_HUD::DrawRequestsDebug()
{
	const AEmbersGameplayTestGameMode* GameMode = GetWorld()->GetAuthGameMode<AEmbersGameplayTestGameMode>();
	if (!GameMode)
	{
		return;
	}

	UFont* Font = GEngine->GetSmallFont();
	const float LineHeight = Font->GetMaxCharHeight() + 4.0f;
	const float StickerSize = LineHeight - 6.0f;
	const double Now = GetWorld()->GetTimeSeconds();
	const TArray<FEGT_Request>& Requests = GameMode->GetRequests();

	float X = RequestsDebugOrigin.X;
	float Y = RequestsDebugOrigin.Y;
	DrawText(FString::Printf(TEXT("REQUESTS (%d)  SCORE: %d"), Requests.Num(), GameMode->GetScore()), FLinearColor::White, X, Y, Font);
	Y += LineHeight;

	for (int32 RequestIndex = 0; RequestIndex < Requests.Num(); ++RequestIndex)
	{
		const FEGT_Request& Request = Requests[RequestIndex];
		const int32 Points = GameMode->GetRequestCurrentPoints(Request);

		// State line: pending (grey), running (green), expired at min points (red), done (cyan), failed (red)
		FString StateLine;
		FLinearColor StateColor;
		if (Request.Status == EEGT_RequestStatus::Succeeded)
		{
			StateLine = FString::Printf(TEXT("#%d  DONE  +%d pts"), RequestIndex, Request.AwardedPoints);
			StateColor = FLinearColor(0.0f, 1.0f, 1.0f);
		}
		else if (Request.Status == EEGT_RequestStatus::Failed)
		{
			StateLine = FString::Printf(TEXT("#%d  FAILED"), RequestIndex);
			StateColor = FLinearColor::Red;
		}
		else if (!Request.HasStarted())
		{
			StateLine = FString::Printf(TEXT("#%d  PENDING  %d pts"), RequestIndex, Points);
			StateColor = FLinearColor::Gray;
		}
		else
		{
			const double ElapsedTime = Now - Request.StartTimestamp;
			const bool bExpired = ElapsedTime >= Request.MaxTime;
			StateLine = FString::Printf(TEXT("#%d  %s  %.1f / %.1f s  %d pts  [%d - %d]"), RequestIndex, bExpired ? TEXT("EXPIRED") : TEXT("RUNNING"),
				FMath::Min(ElapsedTime, static_cast<double>(Request.MaxTime)), Request.MaxTime, Points, Request.MinPoints, Request.MaxPoints);
			StateColor = bExpired ? FLinearColor::Red : FLinearColor::Green;
		}

		if (Request.bIsDoable)
		{
			StateLine += TEXT("  DOABLE");
		}

		if (!Request.bIsCustomRequest)
		{
			StateLine += FString::Printf(TEXT("  (from %s)"), *GetNameSafe(Request.ParcelDefinition));
		}

		DrawText(StateLine, StateColor, X, Y, Font);
		Y += LineHeight;

		// Stickers line: a swatch of the sticker's colour followed by its name
		float StickerX = X + 16.0f;
		for (const EEGT_StickerColor StickerColor : Request.RequiredStickers)
		{
			DrawRect(UEGT_GameplayLibrary::GetStickerLinearColor(this, StickerColor), StickerX, Y + 3.0f, StickerSize, StickerSize);
			StickerX += StickerSize + 4.0f;

			const FString StickerName = UEnum::GetDisplayValueAsText(StickerColor).ToString();
			DrawText(StickerName, FLinearColor::White, StickerX, Y, Font);

			float TextWidth = 0.0f;
			float TextHeight = 0.0f;
			GetTextSize(StickerName, TextWidth, TextHeight, Font);
			StickerX += TextWidth + 12.0f;
		}

		if (Request.RequiredStickers.IsEmpty())
		{
			DrawText(TEXT("(no sticker)"), FLinearColor::Gray, StickerX, Y, Font);
		}
		Y += LineHeight + 4.0f;
	}
}

#pragma endregion
