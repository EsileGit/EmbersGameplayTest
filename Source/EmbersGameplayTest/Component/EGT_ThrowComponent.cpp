// Elise Rosati


#include "Component/EGT_ThrowComponent.h"

#include "EmbersGameplayTest.h"
#include "EmbersGameplayTestCharacter.h"
#include "InteractionComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Curves/CurveFloat.h"

// ---------------------------------------------------------------------------------------------------------------------
UEGT_ThrowComponent::UEGT_ThrowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ThrowComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AEmbersGameplayTestCharacter* Character = Cast<AEmbersGameplayTestCharacter>(GetOwner()))
	{
		CameraComponent = Character->GetFirstPersonCameraComponent();
	}

	InteractionComponent = GetOwner()->FindComponentByClass<UInteractionComponent>();
	if (!InteractionComponent)
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("'%s' has no InteractionComponent: throwing will never be available."), *GetNameSafe(GetOwner()));
		return;
	}

	InteractionComponent->OnHeldItemChanged.AddUniqueDynamic(this, &ThisClass::HandleHeldItemChanged);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ThrowComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InteractionComponent)
	{
		InteractionComponent->OnHeldItemChanged.RemoveDynamic(this, &ThisClass::HandleHeldItemChanged);
	}

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ThrowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ChargeTime += DeltaTime;
	UpdateChargeAlpha();
}

#pragma region Throw

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ThrowComponent::StartCharging()
{
	if (bIsCharging || !InteractionComponent || !InteractionComponent->IsHoldingItem())
	{
		return;
	}

	bIsCharging = true;
	ChargeTime = 0.0f;
	UpdateChargeAlpha();
	SetComponentTickEnabled(true);

	OnThrowChargeStarted(InteractionComponent->GetHeldActor());
	OnThrowChargeChanged.Broadcast(true);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ThrowComponent::ReleaseThrow()
{
	if (!bIsCharging || !InteractionComponent)
	{
		return;
	}

	AActor* Item = InteractionComponent->GetHeldActor();
	const float Strength = ChargeAlpha;

	// The drop broadcast is synchronous: once it returns, the parcel simulates physics again with the carrier velocity
	bIsReleasing = true;
	InteractionComponent->DropHeldItem();
	bIsReleasing = false;

	ApplyThrow(Item, Strength);

	UE_LOG(LogEmbersGameplayTest, Log, TEXT("[Throw] '%s' thrown with strength %.2f"), *GetNameSafe(Item), Strength);

	StopCharging();
	OnThrowReleased(Item, Strength);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ThrowComponent::CancelCharge()
{
	if (!bIsCharging)
	{
		return;
	}

	AActor* Item = InteractionComponent ? InteractionComponent->GetHeldActor() : nullptr;

	StopCharging();
	OnThrowCancelled(Item);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ThrowComponent::StopCharging()
{
	bIsCharging = false;
	ChargeTime = 0.0f;
	ChargeAlpha = 0.0f;
	SetComponentTickEnabled(false);

	OnThrowChargeChanged.Broadcast(false);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ThrowComponent::HandleHeldItemChanged(AActor* HeldItem)
{
	// The held item has been dropped, swapped or consumed by something else than our throw
	if (bIsCharging && !bIsReleasing)
	{
		CancelCharge();
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ThrowComponent::UpdateChargeAlpha()
{
	// Triangle wave: 0 -> 1 in ChargeDuration, then back to 0, and so on
	const float Phase = FMath::Fmod(ChargeTime / ChargeDuration, 2.0f);
	const float RawAlpha = Phase <= 1.0f ? Phase : 2.0f - Phase;

	ChargeAlpha = ChargeCurve ? FMath::Clamp(ChargeCurve->GetFloatValue(RawAlpha), 0.0f, 1.0f) : RawAlpha;
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_ThrowComponent::ApplyThrow(AActor* Item, float Strength) const
{
	if (!IsValid(Item) || !CameraComponent)
	{
		return;
	}

	UPrimitiveComponent* Body = Cast<UPrimitiveComponent>(Item->GetRootComponent());
	if (!Body || !Body->IsSimulatingPhysics())
	{
		return;
	}

	const float Mass = Body->GetMass();

	// Throw upwards
	const FVector Direction = CameraComponent->GetForwardVector().RotateAngleAxis(-ThrowPitchOffset, CameraComponent->GetRightVector());

	// Heavier than the reference mass flies slower, lighter flies faster (within the clamps)
	// This allows to be in control of gameplay (or rather, the limits). A more systemic approach could simply let
	// the physics be completely mass-driven.
	// If we want a very heavy parcel to still be thrown in a range, the first "mapping" should use MinThrowSpeed + SmallValue
	// so that a parcel at a weight above the max still gets the "gauge" gameplay. Another way is simply to set the HeavyMassKg way above the heavier parcel weight.
	const float MaxSpeedForMass = FMath::GetMappedRangeValueClamped(
		 FVector2f(LightMassKg, HeavyMassKg), FVector2f(MaxThrowSpeed, MinThrowSpeed), Mass);
	const float Speed = FMath::Lerp(MinThrowSpeed, MaxSpeedForMass, Strength);

	// Velocity change: stacks on top of the carrier velocity the parcel inherited on drop
	Body->AddImpulse(Direction * Speed, NAME_None, true);
}

#pragma endregion
