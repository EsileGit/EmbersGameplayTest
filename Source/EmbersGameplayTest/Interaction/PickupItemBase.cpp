// Copyright Epic Games, Inc. All Rights Reserved.

#include "PickupItemBase.h"
#include "InteractionTypes.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EmbersGameplayTest.h"

APickupItemBase::APickupItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// the interaction collider is the root so the trace hit always maps back to this actor
	InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Collision"));
	InteractionCollision->SetSphereRadius(InteractionRadius);
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionObjectType(COLLISION_PICKUPITEM);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);

	RootComponent = InteractionCollision;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item Mesh"));
	ItemMesh->SetupAttachment(InteractionCollision);
	ItemMesh->SetCollisionProfileName(FName("NoCollision"));

	InteractionPrompt = NSLOCTEXT("Interaction", "DefaultPickupPrompt", "Pick Up");
}

void APickupItemBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// keep the collider in sync with the radius set on the instance
	InteractionCollision->SetSphereRadius(InteractionRadius);
}

bool APickupItemBase::CanInteract_Implementation(AActor* Interactor)
{
	return bInteractionEnabled;
}

void APickupItemBase::OnInteract_Implementation(AActor* Interactor)
{
	UE_LOG(LogEmbersGameplayTest, Verbose, TEXT("'%s' was interacted with by '%s'"), *GetNameSafe(this), *GetNameSafe(Interactor));
}

bool APickupItemBase::CanBeGrabbed_Implementation(AActor* Interactor)
{
	return bCanBeGrabbed && bInteractionEnabled;
}

FText APickupItemBase::GetInteractionPrompt_Implementation()
{
	return InteractionPrompt;
}

void APickupItemBase::OnFocusBegin_Implementation(AActor* Interactor)
{
}

void APickupItemBase::OnFocusEnd_Implementation(AActor* Interactor)
{
}

void APickupItemBase::ConsumeItem()
{
	Destroy();
}

void APickupItemBase::SetInteractionEnabled(bool bEnabled)
{
	bInteractionEnabled = bEnabled;
}
