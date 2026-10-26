// Elise Rosati


#include "Gameplay/EGT_SnapSpotComponent.h"

#include "Item/EGT_ParcelBase.h"

#include "Components/BoxComponent.h"

// ---------------------------------------------------------------------------------------------------------------------
UEGT_SnapSpotComponent::UEGT_SnapSpotComponent()
{
	SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set arrow default size
	ArrowLength = 40.0f;
	ArrowSize = 0.3f;
	ArrowColor = FColor::Green;

	SetHiddenInGame(true);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_SnapSpotComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (!IsValid(SnapZone))
	{
		// The Trigger profile overlaps the PickupItem channel, which parcels use
		SnapZone = NewObject<UBoxComponent>(Owner, NAME_None, RF_Transient | RF_TextExportTransient);
		SnapZone->SetupAttachment(this);
		SnapZone->SetCollisionProfileName(TEXT("Trigger"));
		SnapZone->ShapeColor = ArrowColor;
		SnapZone->SetHiddenInGame(true);
		SnapZone->RegisterComponent();
	}

	// Re-applied on every register so that edits in the details panel are reflected
	SnapZone->SetBoxExtent(SnapZoneExtent);
	SnapZone->SetRelativeLocation(SnapZoneOffset);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_SnapSpotComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (IsValid(SnapZone))
	{
		SnapZone->DestroyComponent();
	}
	SnapZone = nullptr;

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_SnapSpotComponent::BeginPlay()
{
	Super::BeginPlay();

	if (SnapZone)
	{
		SnapZone->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleSnapZoneBeginOverlap);
		SnapZone->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::HandleSnapZoneEndOverlap);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
bool UEGT_SnapSpotComponent::IsOccupied() const
{
	const AEGT_ParcelBase* Parcel = Occupant.Get();
	return IsValid(Parcel) && !Parcel->IsHeld() && SnapZone->IsOverlappingActor(Parcel);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_SnapSpotComponent::HandleSnapZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Held parcels don't generate overlaps (physics only collision): they are checked once dropped or thrown
	// A parcel already belonging to a spot is ignored, so that a neighbor spot doesn't steal it while it's being snapped
	AEGT_ParcelBase* Parcel = Cast<AEGT_ParcelBase>(OtherActor);
	if (!IsValid(Parcel) || Parcel->IsHeld() || Parcel->GetSnapSpot() || IsOccupied())
	{
		return;
	}

	Occupant = Parcel;
	Parcel->SnapTo(this);
	OnParcelSnapped.Broadcast(Parcel);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_SnapSpotComponent::HandleSnapZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AEGT_ParcelBase* Parcel = Cast<AEGT_ParcelBase>(OtherActor);
	if (!Parcel || Parcel != Occupant.Get())
	{
		return;
	}

	Occupant.Reset();
	Parcel->ClearSnapSpot(this);
}
