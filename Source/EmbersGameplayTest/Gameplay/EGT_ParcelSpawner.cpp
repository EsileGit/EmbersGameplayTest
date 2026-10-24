// Elise Rosati


#include "Gameplay/EGT_ParcelSpawner.h"

#include "EmbersGameplayTest.h"
#include "Item/EGT_ParcelBase.h"
#include "Item/EGT_ParcelDefinition.h"

#include "Components/ArrowComponent.h"
#include "Engine/World.h"

// ---------------------------------------------------------------------------------------------------------------------
AEGT_ParcelSpawner::AEGT_ParcelSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	// Location and orientation of the spawned parcels
	SpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("Spawn Point"));
	SpawnPoint->SetupAttachment(GetRootComponent());
}

#pragma region Spawn

// ---------------------------------------------------------------------------------------------------------------------
AEGT_ParcelBase* AEGT_ParcelSpawner::SpawnParcel()
{
	UEGT_ParcelDefinition* PickedDefinition = PickWeightedDefinition();
	if (!PickedDefinition)
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[Spawner] '%s' has no valid spawn entry"), *GetActorNameOrLabel());
		return nullptr;
	}

	return SpawnParcelOfType(PickedDefinition);
}

// ---------------------------------------------------------------------------------------------------------------------
AEGT_ParcelBase* AEGT_ParcelSpawner::SpawnParcelOfType(UEGT_ParcelDefinition* ParcelDefinition)
{
	if (!ParcelDefinition || !ParcelDefinition->ParcelClass)
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[Spawner] '%s' can't spawn: definition '%s' is missing or has no ParcelClass set."),
			*GetActorNameOrLabel(), *GetNameSafe(ParcelDefinition));
		return nullptr;
	}

	// Deferred spawning so the definition is known before BeginPlay, where the parcel sets up
	AEGT_ParcelBase* Parcel = GetWorld()->SpawnActorDeferred<AEGT_ParcelBase>(ParcelDefinition->ParcelClass, SpawnPoint->GetComponentTransform(),
		this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!Parcel)
	{
		return nullptr;
	}

	Parcel->SetDefinition(ParcelDefinition);
	Parcel->FinishSpawning(SpawnPoint->GetComponentTransform());

	UE_LOG(LogEmbersGameplayTest, Log, TEXT("[Spawner] '%s' spawned '%s' (%s)"), 
		*GetActorNameOrLabel(), *Parcel->GetActorNameOrLabel(), *GetNameSafe(ParcelDefinition));

	OnParcelSpawned.Broadcast(Parcel);
	OnParcelSpawnedFX(Parcel);

	return Parcel;
}

// ---------------------------------------------------------------------------------------------------------------------
UEGT_ParcelDefinition* AEGT_ParcelSpawner::PickWeightedDefinition() const
{
	float TotalWeight = 0.0f;
	for (const FEGT_ParcelSpawnEntry& Entry : SpawnEntries)
	{
		if (Entry.Definition)
		{
			TotalWeight += Entry.Weight;
		}
	}

	if (TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	// Align all the entries and find which one the draw falls in
	float Draw = FMath::FRand() * TotalWeight;
	UEGT_ParcelDefinition* LastValidDefinition = nullptr;
	for (const FEGT_ParcelSpawnEntry& Entry : SpawnEntries)
	{
		if (!Entry.Definition)
		{
			continue;
		}

		LastValidDefinition = Entry.Definition;
		Draw -= Entry.Weight;
		if (Draw < 0.0f)
		{
			return Entry.Definition;
		}
	}

	return LastValidDefinition;
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_ParcelSpawner::DebugSpawnParcel()
{
	if (!GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	SpawnParcel();
}

#pragma endregion
