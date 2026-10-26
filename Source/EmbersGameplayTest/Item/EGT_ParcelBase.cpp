// Elise Rosati


#include "Item/EGT_ParcelBase.h"

#include "EmbersGameplayTest.h"
#include "EmbersGameplayTestGameMode.h"
#include "InteractionComponent.h"
#include "InteractionTypes.h"
#include "Item/EGT_DamageDecalComponent.h"
#include "Item/EGT_ParcelDefinition.h"
#include "Item/EGT_Sticker.h"
#include "Item/EGT_StickerLocationComponent.h"

#include "Algo/RandomShuffle.h"
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

	Integrity = MaxIntegrity;
	RefreshDamageDecals();

	// Drafted at runtime so that every instance of the same definition gets its own weight
	if (Definition)
	{
		const float MassKg = Definition->RollMassKg();
		ParcelMesh->SetMassOverrideInKg(NAME_None, MassKg, true);
		UE_LOG(LogEmbersGameplayTest, Verbose, TEXT("[Parcel_BeginPlay] '%s' (%s) drafted mass: %.1f kg"), *GetActorNameOrLabel(), *GetNameSafe(Definition), MassKg);

		SpawnStickers();
	}

	// Event called when a component hits (or is hit by) something solid
	ParcelMesh->OnComponentHit.AddUniqueDynamic(this, &ThisClass::HandleParcelHit);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Attached actors are not destroyed along with their parent
	for (AEGT_Sticker* Sticker : Stickers)
	{
		if (IsValid(Sticker))
		{
			Sticker->Destroy();
		}
	}
	Stickers.Reset();

	// Whether delivered elsewhere or lost, this parcel no longer answers its request: another one can be made for it
	if (ClaimedRequestIndex != INDEX_NONE)
	{
		if (AEmbersGameplayTestGameMode* GameMode = GetWorld()->GetAuthGameMode<AEmbersGameplayTestGameMode>())
		{
			GameMode->ReleaseRequestForParcel(ClaimedRequestIndex);
		}
		ClaimedRequestIndex = INDEX_NONE;
	}

	Super::EndPlay(EndPlayReason);
}

#pragma region Stickers

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::SpawnStickers()
{
	if (!StickerClass)
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[Parcel::Stickers] '%s' has no StickerClass set: no sticker spawned"), *GetActorNameOrLabel());
		return;
	}

	// Randomize so that stickers land on different faces from one instance to the other
	TArray<UEGT_StickerLocationComponent*> Locations;
	GetComponents<UEGT_StickerLocationComponent>(Locations);
	Algo::RandomShuffle(Locations);

	const TArray<EEGT_StickerColor> Colors = PickStickerColors(Locations.Num());
	const int32 RequestedCount = Colors.Num();
	if (RequestedCount <= 0)
	{
		return;
	}

	if (RequestedCount > Locations.Num())
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[Parcel::Stickers] '%s' (%s) drafted %d stickers but only has %d sticker locations. Additionnal stickers will be ignored"),
			*GetActorNameOrLabel(), *GetNameSafe(Definition), RequestedCount, Locations.Num());
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const int32 StickerCount = FMath::Min(RequestedCount, Locations.Num());
	for (int32 Index = 0; Index < StickerCount; ++Index)
	{
		UEGT_StickerLocationComponent* Location = Locations[Index];
		AEGT_Sticker* Sticker = GetWorld()->SpawnActor<AEGT_Sticker>(StickerClass, Location->GetComponentTransform(), SpawnParams);
		if (!Sticker)
		{
			continue;
		}

		Sticker->AttachToComponent(Location, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Sticker->SetStickerColor(Colors[Index]);
		Stickers.Add(Sticker);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
TArray<EEGT_StickerColor> AEGT_ParcelBase::PickStickerColors(int32 LocationCount)
{
	if (Definition->RollUsesLevelRequirement())
	{
		AEmbersGameplayTestGameMode* GameMode = GetWorld()->GetAuthGameMode<AEmbersGameplayTestGameMode>();
		const int32 RequestIndex = GameMode ? GameMode->ClaimRequestForParcel(LocationCount) : INDEX_NONE;
		if (const FEGT_Request* Request = GameMode ? GameMode->GetRequest(RequestIndex) : nullptr)
		{
			UE_LOG(LogEmbersGameplayTest, Verbose, TEXT("[Parcel::Stickers] '%s' (%s) made for request %d"),
				*GetActorNameOrLabel(), *GetNameSafe(Definition), RequestIndex);
			ClaimedRequestIndex = RequestIndex;
			return Request->RequiredStickers;
		}

		UE_LOG(LogEmbersGameplayTest, Verbose, TEXT("[Parcel::Stickers] '%s' (%s) found no request to be made for: falling back to random stickers"),
			*GetActorNameOrLabel(), *GetNameSafe(Definition));
	}

	if (Definition->StickerColors.IsEmpty())
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[Parcel::Stickers] '%s' (%s) has no StickerColors to draft from: no sticker spawned"),
			*GetActorNameOrLabel(), *GetNameSafe(Definition));
	}
	return Definition->RollStickerColors();
}

// ---------------------------------------------------------------------------------------------------------------------
TArray<EEGT_StickerColor> AEGT_ParcelBase::GetStickerColors() const
{
	TArray<EEGT_StickerColor> Colors;
	for (const AEGT_Sticker* Sticker : Stickers)
	{
		if (IsValid(Sticker))
		{
			Colors.Add(Sticker->GetStickerColor());
		}
	}
	return Colors;
}

#pragma endregion

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

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::SetFrozen(bool bFrozen)
{
	if (bFrozen)
	{
		ParcelMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		ParcelMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
	ParcelMesh->SetSimulatePhysics(!bFrozen);
	SetInteractionEnabled(!bFrozen);
}

// ---------------------------------------------------------------------------------------------------------------------
FEGT_ParcelInspectInfo AEGT_ParcelBase::GetInspectInfo() const
{
	FEGT_ParcelInspectInfo Info;
	Info.DisplayName = (Definition && !Definition->DisplayName.IsEmpty()) ? Definition->DisplayName : FText::FromString(GetActorNameOrLabel());
	//Info.MassKg = ParcelMesh->GetMass(); // This warns that simulate physics is not enabled. Would require further investigation to see whether the new method to get mass is fine or not
	Info.MassKg = ParcelMesh->CalculateMass();
	Info.DamageState = DamageState;
	Info.IntegrityRatio = GetIntegrityRatio();
	Info.FragilityPercent = FragilityPercent;
	Info.StickerColors = GetStickerColors();

	return Info;
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
	ApplyImpactDamage(ImpactSpeed, Hit);
}

#pragma endregion

#pragma region Damage

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::ApplyImpactDamage(float ImpactSpeed, const FHitResult& Hit)
{
	if (ImpactSpeed < DamageMinSpeed || DamageState == EEGT_ParcelDamageState::Broken)
	{
		return;
	}

	const float BaseDamage = FMath::GetMappedRangeValueClamped(
		FVector2f(DamageMinSpeed, DamageMaxSpeed), FVector2f(MinDamagePerHit, MaxDamagePerHit), ImpactSpeed);
	const float Damage = BaseDamage * (1.0f + FragilityPercent / 100.0f);
	Integrity = FMath::Clamp(Integrity - Damage, 0.0f, MaxIntegrity);

	UE_LOG(LogEmbersGameplayTest, Log, TEXT("[ParcelDamage] '%s' took %.1f damage (base %.1f, fragility %.0f%%) at %.0f cm/s | Integrity: %.1f / %.1f"),
		*GetActorNameOrLabel(), Damage, BaseDamage, FragilityPercent, ImpactSpeed, Integrity, MaxIntegrity);

	OnParcelDamaged(Damage, Integrity, Hit);

	const AEmbersGameplayTestGameMode* GameMode = GetWorld()->GetAuthGameMode<AEmbersGameplayTestGameMode>();
	if (!GameMode)
	{
		return;
	}

	const EEGT_ParcelDamageState OldState = DamageState;
	DamageState = GameMode->GetDamageStateForIntegrity(GetIntegrityRatio());
	if (DamageState != OldState)
	{
		UE_LOG(LogEmbersGameplayTest, Log, TEXT("[ParcelDamage] '%s' is now %s"), *GetActorNameOrLabel(), *UEnum::GetValueAsString(DamageState));
		RefreshDamageDecals();
		// It's possible to "skip" a state, they are not sequential. Reaction should be based on current state and not previous one.
		OnDamageStateChanged(DamageState, OldState);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelBase::RefreshDamageDecals()
{
	TArray<UEGT_DamageDecalComponent*> DamageDecals;
	GetComponents<UEGT_DamageDecalComponent>(DamageDecals);
	for (UEGT_DamageDecalComponent* DamageDecal : DamageDecals)
	{
		DamageDecal->RefreshForDamageState(DamageState);
	}
}

#pragma endregion
