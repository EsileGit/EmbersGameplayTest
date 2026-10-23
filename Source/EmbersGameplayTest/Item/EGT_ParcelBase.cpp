// Elise Rosati


#include "Item/EGT_ParcelBase.h"

#include "EmbersGameplayTest.h"
#include "InteractionComponent.h"
#include "InteractionTypes.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

// ---------------------------------------------------------------------------------------------------------------------
AEGT_ParcelBase::AEGT_ParcelBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// The mesh is both the physical body and what the interaction trace hits
	ParcelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Parcel Mesh"));
	SetRootComponent(ParcelMesh);

	ParcelMesh->SetMobility(EComponentMobility::Movable);
	ParcelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ParcelMesh->SetCollisionObjectType(COLLISION_PICKUPITEM);
	ParcelMesh->SetCollisionResponseToAllChannels(ECR_Block);
	ParcelMesh->SetSimulatePhysics(true);

	// needed for OnComponentHit, which drives the impact feedback
	ParcelMesh->SetNotifyRigidBodyCollision(true);

	// Thin geometry safeguard (probably not needed ?)
	ParcelMesh->BodyInstance.bUseCCD = true;

	InteractionPrompt = NSLOCTEXT("Parcel", "DefaultParcelPrompt", "Take Parcel");
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::BeginPlay()
{
	Super::BeginPlay();

	// Event called when a component hits (or is hit by) something solid
	ParcelMesh->OnComponentHit.AddUniqueDynamic(this, &ThisClass::HandleParcelHit);
}

#pragma region Interaction

// ---------------------------------------------------------------------------------------------------------------------
bool AEGT_ParcelBase::CanInteract_Implementation(AActor* Interactor)
{
	return bInteractionEnabled;
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::OnInteract_Implementation(AActor* Interactor)
{
	UE_LOG(LogEmbersGameplayTest, Verbose, TEXT("'%s' was interacted with by '%s'"), *GetNameSafe(this), *GetNameSafe(Interactor));

	// TryInteract calls OnInteract even on items it won't grab
	if (!IsValid(Interactor) || !IInteractable::Execute_CanBeGrabbed(this, Interactor))
	{
		return;
	}

	UInteractionComponent* InteractionComponent = Interactor->FindComponentByClass<UInteractionComponent>();
	if (!InteractionComponent)
	{
		return;
	}

	HoldingInteractionComponent = InteractionComponent;
	HoldingInteractionComponent->OnHeldItemChanged.AddUniqueDynamic(this, &ThisClass::HandleHeldItemChanged);

	// physics has to be off before GrabItem attaches the parcel to the camera.
	// Physics only collision: the carrier's movement sweep and the interaction trace ignore the held parcel,
	// but it still pushes the parcels it bumps into.
	ParcelMesh->SetSimulatePhysics(false);
	ParcelMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	StopDisplayingHighlight();

	OnParcelGrabbed(Interactor);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::HandleHeldItemChanged(AActor* HeldItem)
{
	if (HeldItem == this)
	{
		// This Parcel is the newly held item
		return;
	}

	// Something else (or nothing) is currently grabbed (aka not this): the parcel has been dropped
	AActor* Carrier = nullptr;
	if (HoldingInteractionComponent)
	{
		Carrier = HoldingInteractionComponent->GetOwner();
		HoldingInteractionComponent->OnHeldItemChanged.RemoveDynamic(this, &ThisClass::HandleHeldItemChanged);
		HoldingInteractionComponent = nullptr;
	}

	ParcelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ParcelMesh->SetSimulatePhysics(true);

	// Keep the carrier's momentum to enable "goofy" gameplay (a bit of chaos)
	if (bInheritCarrierVelocityOnDrop && IsValid(Carrier))
	{
		UE_LOG(LogEmbersGameplayTest, Log, TEXT("[Drop] Parcel '%s' dropped at speed %s"), *GetActorNameOrLabel(), *Carrier->GetVelocity().ToCompactString());
		ParcelMesh->SetPhysicsLinearVelocity(Carrier->GetVelocity());
	}

	OnParcelDropped(Carrier);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::StopDisplayingHighlight() const
{
	ParcelMesh->SetRenderCustomDepth(false);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::StartDisplayingHighlight() const
{
	if (bHighlightOnFocus)
	{
		ParcelMesh->SetRenderCustomDepth(true);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
bool AEGT_ParcelBase::CanBeGrabbed_Implementation(AActor* Interactor)
{
	return bCanBeGrabbed && bInteractionEnabled;
}

// ---------------------------------------------------------------------------------------------------------------------
FText AEGT_ParcelBase::GetInteractionPrompt_Implementation()
{
	return InteractionPrompt;
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::OnFocusBegin_Implementation(AActor* Interactor)
{
	StartDisplayingHighlight();
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::OnFocusEnd_Implementation(AActor* Interactor)
{
	StopDisplayingHighlight();
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::ConsumeItem()
{
	Destroy();
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::SetInteractionEnabled(bool bEnabled)
{
	bInteractionEnabled = bEnabled;
}

#pragma endregion

#pragma region Feel

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::HandleParcelHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (false)
	{
		UE_LOG(LogEmbersGameplayTest, Log,
			TEXT("[ParcelHit] '%s' hit '%s' (%s) | Simulating: %d | NormalImpulse: %s (size %.0f) | Mass: %.1f kg | dv: %.0f cm/s | ImpactNormal: %s"),
			*GetActorNameOrLabel(),
			*GetNameSafe(OtherActor),
			*GetNameSafe(OtherComp),
			ParcelMesh->IsSimulatingPhysics(),
			*NormalImpulse.ToCompactString(),
			NormalImpulse.Size(),
			ParcelMesh->GetMass(),
			ParcelMesh->GetMass() > 0.0f ? NormalImpulse.Size() / ParcelMesh->GetMass() : 0.0f,
			*Hit.ImpactNormal.ToCompactString());
	}
	

	
	// No damage while in hands
	if (!ParcelMesh->IsSimulatingPhysics())
	{
		return;
	}

	const float Mass = ParcelMesh->GetMass();
	if (Mass <= 0.0f)
	{
		return;
	}

	// Impulse = mass * dv.
	const float ImpactSpeed = NormalImpulse.Size() / Mass;
	if (ImpactSpeed < MinImpactSpeed)
	{
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastImpactTime < ImpactCooldown)
	{
		return;
	}
	LastImpactTime = Now;

	// 0 at MinImpactSpeed, 1 at MaxImpactSpeed
	const float ImpactIntensity = FMath::GetMappedRangeValueClamped(
		FVector2f(MinImpactSpeed, MaxImpactSpeed), FVector2f(0.0f, 1.0f), ImpactSpeed);

	UE_LOG(LogEmbersGameplayTest, Log,
		TEXT("[Registered ParcelHit] '%s' hit '%s' (%s) | NormalImpulse: %s (size %.0f) | Mass: %.1f kg | dv: %.0f cm/s (min: %.0f) | Intensity: %.2f | ImpactNormal: %s"),
		*GetActorNameOrLabel(),
		*GetNameSafe(OtherActor),
		*GetNameSafe(OtherComp),
		*NormalImpulse.ToCompactString(),
		NormalImpulse.Size(),
		ParcelMesh->GetMass(),
		ImpactSpeed,
		MinImpactSpeed,
		ImpactIntensity,
		*Hit.ImpactNormal.ToCompactString());

	OnParcelImpact(ImpactSpeed, ImpactIntensity, Hit);
}

#pragma endregion
