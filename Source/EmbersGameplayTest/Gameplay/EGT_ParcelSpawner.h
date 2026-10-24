// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EGT_ParcelSpawner.generated.h"

class AEGT_ParcelBase;
class UArrowComponent;
class UEGT_ParcelDefinition;

/** Broadcast every time the spawner creates a parcel */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParcelSpawned, AEGT_ParcelBase*, Parcel);

/**
 * A parcel type for the spawner and the weight for its choice (no max, they simply "concurentiate" each other)
 */
USTRUCT(BlueprintType)
struct FEGT_ParcelSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	TObjectPtr<UEGT_ParcelDefinition> Definition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn", meta = (ClampMin = 0))
	float Weight = 1.0f;
};

/**
 * Spawns parcels on demand at its spawn point. The parcel type is picked from a weighted list of definitions.
 */
UCLASS(Abstract, Blueprintable)
class EMBERSGAMEPLAYTEST_API AEGT_ParcelSpawner : public AActor
{
	GENERATED_BODY()

public:
	AEGT_ParcelSpawner();

	// Fired after a parcel has been spawned and set up (can be used for gameplay)
	UPROPERTY(BlueprintAssignable, Category="Spawn")
	FOnParcelSpawned OnParcelSpawned;

	
	UFUNCTION(BlueprintCallable, Category="Spawn")
	AEGT_ParcelBase* SpawnParcel();

	UFUNCTION(BlueprintCallable, Category="Spawn")
	AEGT_ParcelBase* SpawnParcelOfType(UEGT_ParcelDefinition* ParcelDefinition);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UArrowComponent> SpawnPoint;

#pragma region Spawn

	// Picks a definition from SpawnEntries according to their weights
	UEGT_ParcelDefinition* PickWeightedDefinition() const;

	// Details panel button, to test spawning from editor directly
	UFUNCTION(CallInEditor, Category="Debug")
	void DebugSpawnParcel();

	// PROPERTIES
	// Parcel types this spawner can pick from
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	TArray<FEGT_ParcelSpawnEntry> SpawnEntries;

#pragma endregion

#pragma region GameFeel

	/** Blueprint reactions hook (for cosmetic) (Redundant with OnParcelSpawned but it seems cleaner to separate cosmetic stuff from gameppla)*/
	UFUNCTION(BlueprintImplementableEvent, Category="Spawn")
	void OnParcelSpawnedFX(AEGT_ParcelBase* Parcel);

#pragma endregion
};
