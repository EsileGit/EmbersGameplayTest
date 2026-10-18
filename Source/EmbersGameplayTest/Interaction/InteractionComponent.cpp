// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractionComponent.h"
#include "Interactable.h"
#include "InteractionTypes.h"
#include "EmbersGameplayTestCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// cache the first person camera we trace from
	if (AEmbersGameplayTestCharacter* Character = Cast<AEmbersGameplayTestCharacter>(GetOwner()))
	{
		CameraComponent = Character->GetFirstPersonCameraComponent();
	}
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateFocus();
}

void UInteractionComponent::UpdateFocus()
{
	FHitResult Hit;

	if (TraceForInteractable(Hit))
	{
		AActor* HitActor = Hit.GetActor();

		// Ici on peut économiser le cast (on n'appelle pas l'execute sur le Interactable mais sur l'Uobject directmeent)
		// if (IInteractable* Interactable = Cast<IInteractable>(HitActor))
		if (!(IsValid(HitActor) && HitActor->Implements<UInteractable>()))
		{
			// Ici on s'attend à ce que HitActor soit toujours valide, car TraceForInteractable renvoie bHit. Mais
			// le check de validité permet de s'assurer que le code ne peut pas crasher dans un cas innattendu 
			return;
		}
		
		if (IInteractable::Execute_CanInteract(HitActor, GetOwner()))
		{
			SetFocusedActor(HitActor);
		}
		
	}
}

bool UInteractionComponent::TraceForInteractable(FHitResult& OutHit) const
{
	if (!CameraComponent)
	{
		return false;
	}

	const FVector TraceStart = CameraComponent->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (GetOwner()->GetActorForwardVector() * InteractionDistance);

	FCollisionQueryParams QueryParams(FName(TEXT("InteractionTrace")), false);
	QueryParams.AddIgnoredActor(GetOwner());

	// only look for actors sitting on the pickup item object channel
	const FCollisionObjectQueryParams ObjectQueryParams(COLLISION_PICKUPITEM);

	const bool bHit = GetWorld()->LineTraceSingleByObjectType(OutHit, TraceStart, TraceEnd, ObjectQueryParams, QueryParams);

	if (bDrawDebugTrace)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, -1.0f, 0, 1.0f);
	}

	return bHit;
}

void UInteractionComponent::SetFocusedActor(AActor* NewFocusedActor)
{
	if (NewFocusedActor == FocusedActor)
	{
		return;
	}

	AActor* PreviousFocusedActor = FocusedActor;

	if (IsValid(PreviousFocusedActor) && PreviousFocusedActor->Implements<UInteractable>())
	{
		IInteractable::Execute_OnFocusEnd(PreviousFocusedActor, GetOwner());
	}

	FocusedActor = NewFocusedActor;

	if (IsValid(FocusedActor) && FocusedActor->Implements<UInteractable>())
	{
		IInteractable::Execute_OnFocusBegin(FocusedActor, GetOwner());
	}

	OnInteractionFocusChanged.Broadcast(FocusedActor, PreviousFocusedActor);
}

void UInteractionComponent::TryInteract()
{
	// interacting again while carrying something puts it back down
	if (HeldActor)
	{
		DropHeldItem();
		return;
	}

	if (!(IsValid(FocusedActor) && FocusedActor->Implements<UInteractable>()))
	{
		return;
	}

	IInteractable::Execute_OnInteract(FocusedActor, GetOwner());

	if (IInteractable::Execute_CanBeGrabbed(FocusedActor, GetOwner()))
	{
		GrabItem(FocusedActor);
	}
}

void UInteractionComponent::GrabItem(AActor* ItemToGrab)
{
	if (!ItemToGrab || !CameraComponent)
	{
		return;
	}

	ItemToGrab->AttachToComponent(CameraComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	ItemToGrab->SetActorRelativeLocation(HeldItemOffset);
	ItemToGrab->SetActorRelativeRotation(FRotator::ZeroRotator);

	HeldActor = ItemToGrab;

	OnHeldItemChanged.Broadcast(HeldActor);
}

void UInteractionComponent::DropHeldItem()
{
	if (!HeldActor)
	{
		return;
	}

	AActor* DroppedActor = HeldActor;

	DroppedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	HeldActor = nullptr;

	OnHeldItemChanged.Broadcast(nullptr);
}

FText UInteractionComponent::GetFocusedInteractionPrompt() const
{
	if (IsValid(FocusedActor) && FocusedActor->Implements<UInteractable>())
	{
		return IInteractable::Execute_GetInteractionPrompt(FocusedActor);
	}

	return FText::GetEmpty();
}
