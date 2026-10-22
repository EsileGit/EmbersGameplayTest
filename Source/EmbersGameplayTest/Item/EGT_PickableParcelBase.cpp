// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/EGT_PickableParcelBase.h"

#include "InteractionComponent.h"
#include "InteractionTypes.h"

#include "Components/SphereComponent.h"

// ---------------------------------------------------------------------------------------------------------------------
AEGT_PickableParcelBase::AEGT_PickableParcelBase()
{
	UStaticMeshComponent* Mesh = GetItemMesh();
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//Mesh->SetSimulatePhysics(true);
	GetInteractionCollision()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetInteractionCollision()->SetSimulatePhysics(true);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_PickableParcelBase::OnInteract_Implementation(AActor* Interactor)
{
	Super::OnInteract_Implementation(Interactor);
	
	// TryInteract calls OnInteract even on items it won't grab (the event wouldn't be called so it wouldn't be a 
	// problem, but there would be no need for any of the following so...
	if (!IsValid(Interactor) || !IInteractable::Execute_CanBeGrabbed(this, Interactor))
	{
		return;
	}

	UInteractionComponent* InteractionComponent = Interactor->FindComponentByClass<UInteractionComponent>();
	if (!InteractionComponent)
	{
		return;
	}

	// physics has to be off before GrabItem attaches the parcel to the camera
	//GetItemMesh()->SetSimulatePhysics(false);
	GetInteractionCollision()->SetSimulatePhysics(false);
	// The pawn should not be blocked by the grabbed object
	GetItemMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	HoldingInteractionComponent = InteractionComponent;
	HoldingInteractionComponent->OnHeldItemChanged.AddUniqueDynamic(this, &ThisClass::HandleHeldItemChanged);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_PickableParcelBase::HandleHeldItemChanged(AActor* HeldItem)
{
	if (HeldItem == this)
	{
		// This Parcel is the newly held item
		return;
	}

	// Something else (or nothing) is currently grabbed (aka not this)
	if (HoldingInteractionComponent)
	{
		HoldingInteractionComponent->OnHeldItemChanged.RemoveDynamic(this, &ThisClass::HandleHeldItemChanged);
		HoldingInteractionComponent = nullptr;
	}

	//GetItemMesh()->SetSimulatePhysics(true);
	GetInteractionCollision()->SetSimulatePhysics(true);
	GetItemMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}
