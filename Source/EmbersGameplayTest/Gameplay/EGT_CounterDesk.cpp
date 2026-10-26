// Elise Rosati


#include "Gameplay/EGT_CounterDesk.h"

#include "EmbersGameplayTest.h"
#include "EmbersGameplayTestGameMode.h"
#include "Item/EGT_ParcelBase.h"

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"

// ---------------------------------------------------------------------------------------------------------------------
AEGT_CounterDesk::AEGT_CounterDesk()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// The Trigger profile overlaps the PickupItem channel, which parcels use
	DeliveryZone = CreateDefaultSubobject<UBoxComponent>(TEXT("Delivery Zone"));
	DeliveryZone->SetupAttachment(Root);
	DeliveryZone->SetCollisionProfileName(TEXT("Trigger"));
	DeliveryZone->SetBoxExtent(FVector(40.0f, 40.0f, 20.0f));
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_CounterDesk::BeginPlay()
{
	Super::BeginPlay();

	DeliveryZone->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleDeliveryZoneOverlap);
}

// ---------------------------------------------------------------------------------------------------------------------
AEmbersGameplayTestGameMode* AEGT_CounterDesk::GetGameMode() const
{
	return GetWorld()->GetAuthGameMode<AEmbersGameplayTestGameMode>();
}

// ---------------------------------------------------------------------------------------------------------------------
bool AEGT_CounterDesk::GetActiveRequest(FEGT_Request& OutRequest) const
{
	const AEmbersGameplayTestGameMode* GameMode = GetGameMode();
	const FEGT_Request* Request = GameMode ? GameMode->GetRequest(ActiveRequestIndex) : nullptr;
	if (!Request)
	{
		return false;
	}

	OutRequest = *Request;
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool AEGT_CounterDesk::GetActiveRequestProgress(float& OutRemainingTime, int32& OutCurrentPoints) const
{
	const AEmbersGameplayTestGameMode* GameMode = GetGameMode();
	const FEGT_Request* Request = GameMode ? GameMode->GetRequest(ActiveRequestIndex) : nullptr;
	if (!Request)
	{
		return false;
	}

	OutRemainingTime = Request->GetRemainingTimeAt(GetWorld()->GetTimeSeconds());
	OutCurrentPoints = GameMode->GetRequestCurrentPoints(*Request);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool AEGT_CounterDesk::TryDraftRequest()
{
	AEmbersGameplayTestGameMode* GameMode = GetGameMode();
	if (HasActiveRequest() || !GameMode || !GameMode->IsLevelPlaying())
	{
		return false;
	}

	const int32 RequestIndex = GameMode->DraftRequest();
	if (RequestIndex == INDEX_NONE)
	{
		OnNoRequestAvailable();
		return false;
	}

	ActiveRequestIndex = RequestIndex;
	OnRequestDrafted(*GameMode->GetRequest(RequestIndex));
	OnActiveRequestChanged.Broadcast(true);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool AEGT_CounterDesk::CancelActiveRequest()
{
	AEmbersGameplayTestGameMode* GameMode = GetGameMode();
	if (!HasActiveRequest() || !GameMode)
	{
		return false;
	}

	const int32 RequestIndex = ActiveRequestIndex;
	GameMode->CompleteRequest(RequestIndex, false);
	ActiveRequestIndex = INDEX_NONE;

	OnRequestCancelled(*GameMode->GetRequest(RequestIndex));
	OnActiveRequestChanged.Broadcast(false);
	return true;
}

#pragma region Delivery

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_CounterDesk::HandleDeliveryZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Held parcels don't generate overlaps (physics only collision): they are checked once dropped or thrown
	// Once the level is over, late parcels (thrown right before the end) don't count anymore
	AEGT_ParcelBase* Parcel = Cast<AEGT_ParcelBase>(OtherActor);
	const AEmbersGameplayTestGameMode* GameMode = GetGameMode();
	const bool bIsParcelHeld = !IsValid(Parcel) || Parcel->IsHeld();
	const bool bIsLevelPlaying = GameMode && GameMode->IsLevelPlaying();
	if (bIsParcelHeld || !HasActiveRequest() || !bIsLevelPlaying)
	{
		return;
	}

	DeliverParcel(Parcel);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_CounterDesk::DeliverParcel(AEGT_ParcelBase* Parcel)
{
	AEmbersGameplayTestGameMode* GameMode = GetGameMode();
	const FEGT_Request* Request = GameMode ? GameMode->GetRequest(ActiveRequestIndex) : nullptr;
	if (!Request)
	{
		return;
	}

	Parcel->SetFrozen(true);

	if (!Request->MatchesStickers(Parcel->GetStickerColors()))
	{
		UE_LOG(LogEmbersGameplayTest, Log, TEXT("[CounterDesk] '%s' rejected parcel '%s' for request %d"),
			*GetActorNameOrLabel(), *Parcel->GetActorNameOrLabel(), ActiveRequestIndex);

		Parcel->SetFrozen(false);
		OnParcelRejected(Parcel, *Request);
		return;
	}

	const int32 RequestIndex = ActiveRequestIndex;
	const int32 Points = GameMode->CompleteRequest(RequestIndex, true, Parcel->GetDamageState());
	ActiveRequestIndex = INDEX_NONE;

	OnRequestFulfilled(Parcel, *GameMode->GetRequest(RequestIndex), Points);
	OnActiveRequestChanged.Broadcast(false);

	if (ConsumeDelay > 0.0f)
	{
		Parcel->SetLifeSpan(ConsumeDelay);
	}
	else
	{
		Parcel->ConsumeItem();
	}
}

#pragma endregion
