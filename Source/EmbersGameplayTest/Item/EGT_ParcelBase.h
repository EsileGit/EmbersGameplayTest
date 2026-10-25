// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "EGT_ParcelBase.generated.h"

class AEGT_Sticker;
class UEGT_ParcelDefinition;
class UInteractionComponent;
class UStaticMeshComponent;

/**
 * Everything the inspect UI can display about a parcel. Add new fields here (identifier, damage...).
 */
USTRUCT(BlueprintType)
struct FEGT_ParcelInspectInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel", meta = (Units = "kg"))
	float MassKg = 0.0f;
};

/**
 * Base class for parcels the player can look at, interact with and carry around.
 * The item shape (mesh) determines its interaction "zone" and its physics
 */
UCLASS(Abstract, Blueprintable)
class EMBERSGAMEPLAYTEST_API AEGT_ParcelBase : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AEGT_ParcelBase();
	
	// Getter & Setter
	UStaticMeshComponent* GetParcelMesh() const { return ParcelMesh; }

	UFUNCTION(BlueprintPure, Category="Parcel")
	UEGT_ParcelDefinition* GetDefinition() const { return Definition; }
	void SetDefinition(UEGT_ParcelDefinition* InDefinition) { Definition = InDefinition; }

	//~ Begin IInteractable
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanBeGrabbed_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;
	virtual void OnFocusBegin_Implementation(AActor* Interactor) override;
	virtual void OnFocusEnd_Implementation(AActor* Interactor) override;
	//~ End IInteractable

	/** Removes this item from the world. Call it from Blueprint once the item has been consumed. */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void ConsumeItem();

	/** Turns interaction on this parcel on or off */
	UFUNCTION(BlueprintCallable, Category="Parcel")
	void SetInteractionEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="Parcel")
	bool IsHeld() const { return HoldingInteractionComponent != nullptr; }

	/** Gathers the info displayed by the inspect UI */
	UFUNCTION(BlueprintPure, Category="Parcel")
	FEGT_ParcelInspectInfo GetInspectInfo() const;

	UFUNCTION(BlueprintPure, Category="Parcel|Stickers")
	const TArray<AEGT_Sticker*>& GetStickers() const { return Stickers; }


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ParcelMesh;

	// Description sheet of this parcel
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UEGT_ParcelDefinition> Definition;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma region Stickers

	// Spawns one sticker per color in the definition, on randomly picked sticker locations
	void SpawnStickers();

	// PROPERTIES
	UPROPERTY(EditDefaultsOnly, Category="Parcel|Stickers")
	TSubclassOf<AEGT_Sticker> StickerClass;

	// Runtime
	UPROPERTY(Transient)
	TArray<TObjectPtr<AEGT_Sticker>> Stickers;

#pragma endregion

#pragma region Interaction

	// Callback triggered by the carrier's interaction component, used to detect the drop
	UFUNCTION()
	void HandleHeldItemChanged(AActor* HeldItem);

	void StopDisplayingHighlight() const;
	void StartDisplayingHighlight() const;
	
	// PROPERTIES
	/** Text shown on the HUD while the player is looking at this parcel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText InteractionPrompt;

	/** Whether the player can currently interact with this parcel at all */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	bool bInteractionEnabled = true;

	/** Whether interacting with this parcel makes the player carry it around */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	bool bCanBeGrabbed = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	bool bHighlightOnFocus = true;

	// Runtime
	// Interaction component of the player carrying this parcel.
	UPROPERTY(Transient)
	TObjectPtr<UInteractionComponent> HoldingInteractionComponent;

#pragma endregion

#pragma region GameFeel
	
	// Blueprint reactions hook (for cosmetic)
	UFUNCTION(BlueprintImplementableEvent, Category="Parcel")
	void OnParcelGrabbed(AActor* Carrier);

	UFUNCTION(BlueprintImplementableEvent, Category="Parcel")
	void OnParcelDropped(AActor* Carrier);

	UFUNCTION(BlueprintImplementableEvent, Category="Parcel")
	void OnParcelImpact(float ImpactSpeed, float ImpactIntensity, const FHitResult& Hit);

	
	// Callback triggered by the parcel mesh's physics hits
	UFUNCTION()
	void HandleParcelHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);


	// PROPERTIES
	// Whether a parcel keeps its carrier's velocity on drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel|Impact")
	bool bInheritCarrierVelocityOnDrop = true;

	// Minimum velocity needed to register a hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel|Impact", meta = (ClampMin = 0, Units = "CentimetersPerSecond"))
	float MinImpactSpeed = 150.0f;

	// For cosmetic use (for now) : impact speed considered "as violent as it gets"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel|Impact", meta = (ClampMin = 0, Units = "CentimetersPerSecond"))
	float MaxImpactSpeed = 1500.0f;

	// Minimum delay needed between two OnParcelImpact events
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel|Impact", meta = (ClampMin = 0, Units = "s"))
	float ImpactCooldown = 0.1f;
	
	// Runtime
	double LastImpactTime = -1.0;

#pragma endregion

};
