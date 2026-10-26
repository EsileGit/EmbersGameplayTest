// Elise Rosati

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "Item/EGT_ParcelDamageTypes.h"
#include "Item/EGT_StickerTypes.h"
#include "EGT_ParcelBase.generated.h"

class AEGT_Sticker;
class UEGT_ParcelDefinition;
class UEGT_SnapSpotComponent;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel")
	EEGT_ParcelDamageState DamageState = EEGT_ParcelDamageState::Intact;

	// 1 = intact, 0 = no integrity left
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel")
	float IntegrityRatio = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel", meta = (Units = "Percent"))
	float FragilityPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel")
	TArray<EEGT_StickerColor> StickerColors;
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

	/** Get the colors of the stickers on this parcel */
	UFUNCTION(BlueprintPure, Category="Parcel|Stickers")
	TArray<EEGT_StickerColor> GetStickerColors() const;

	UFUNCTION(BlueprintCallable, Category="Parcel")
	void SetFrozen(bool bFrozen);

	UFUNCTION(BlueprintPure, Category="Parcel|Damage")
	float GetIntegrity() const { return Integrity; }

	// 1 = intact, 0 = no integrity left
	UFUNCTION(BlueprintPure, Category="Parcel|Damage")
	float GetIntegrityRatio() const { return MaxIntegrity > 0.0f ? Integrity / MaxIntegrity : 0.0f; }

	UFUNCTION(BlueprintPure, Category="Parcel|Damage")
	EEGT_ParcelDamageState GetDamageState() const { return DamageState; }

	// Teleports the parcel so that its snap attach point lies on the spot's arrow, and stops it
	void SnapTo(UEGT_SnapSpotComponent* Spot);

	// Spot this parcel was snapped on, if it still lies on it
	UFUNCTION(BlueprintPure, Category="Parcel|Snap")
	UEGT_SnapSpotComponent* GetSnapSpot() const;

	// Only clears if Spot is the current one
	void ClearSnapSpot(const UEGT_SnapSpotComponent* Spot);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ParcelMesh;

	// Description sheet of this parcel
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parcel", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UEGT_ParcelDefinition> Definition;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma region Stickers

	// Spawns one sticker per picked color, on randomly picked sticker locations
	void SpawnStickers();

	// Colors for this parcel's stickers, according to the definition's spawn policy
	TArray<EEGT_StickerColor> PickStickerColors(int32 LocationCount);

	// PROPERTIES
	UPROPERTY(EditDefaultsOnly, Category="Parcel|Stickers")
	TSubclassOf<AEGT_Sticker> StickerClass;

	// Runtime
	UPROPERTY(Transient)
	TArray<TObjectPtr<AEGT_Sticker>> Stickers;

	// Request this parcel was made for (FromLevelRequirement policy). This relies on the Request list to never reorder. Probably best to use ID...
	UPROPERTY(Transient, VisibleInstanceOnly, Category="Parcel|Stickers")
	int32 ClaimedRequestIndex = INDEX_NONE;

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

#pragma region Damage

	void ApplyImpactDamage(float ImpactSpeed, const FHitResult& Hit);

	// Shows the damage decals matching the current damage state
	void RefreshDamageDecals();

	// PROPERTIES
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel|Damage", meta = (ClampMin = 1))
	float MaxIntegrity = 100.0f;

	// Extra damage taken: negative values can work
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel|Damage", meta = (Units = "Percent"))
	float FragilityPercent = 0.0f;

	// Impact speed from which the parcel takes damage (~1m fall). Slower impacts are cosmetic only
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel|Damage", meta = (ClampMin = 0, Units = "CentimetersPerSecond"))
	float DamageMinSpeed = 450.0f;

	// Impact speed dealing MaxDamagePerHit (~5m fall)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel|Damage", meta = (ClampMin = 0, Units = "CentimetersPerSecond"))
	float DamageMaxSpeed = 1000.0f;

	// Damage dealt at DamageMinSpeed, before fragility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel|Damage", meta = (ClampMin = 0))
	float MinDamagePerHit = 5.0f;

	// Damage dealt at DamageMaxSpeed and above, before fragility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parcel|Damage", meta = (ClampMin = 0))
	float MaxDamagePerHit = 50.0f;

	// Runtime
	UPROPERTY(Transient, VisibleInstanceOnly, Category="Parcel|Damage")
	float Integrity = 0.0f;

	UPROPERTY(Transient, VisibleInstanceOnly, Category="Parcel|Damage")
	EEGT_ParcelDamageState DamageState = EEGT_ParcelDamageState::Intact;

#pragma endregion

#pragma region Snapping

	// Runtime
	UPROPERTY(Transient, VisibleInstanceOnly, Category="Parcel|Snap")
	TWeakObjectPtr<UEGT_SnapSpotComponent> CurrentSnapSpot;

#pragma endregion

#pragma region GameFeel

	// Blueprint reactions hook (for cosmetic)
	UFUNCTION(BlueprintImplementableEvent, Category="Parcel")
	void OnParcelGrabbed(AActor* Carrier);

	UFUNCTION(BlueprintImplementableEvent, Category="Parcel")
	void OnParcelDropped(AActor* Carrier);

	UFUNCTION(BlueprintImplementableEvent, Category="Parcel")
	void OnParcelImpact(float ImpactSpeed, float ImpactIntensity, const FHitResult& Hit);

	// Called after OnParcelImpact when the impact was strong enough to damage the parcel
	UFUNCTION(BlueprintImplementableEvent, Category="Parcel")
	void OnParcelDamaged(float Damage, float NewIntegrity, const FHitResult& Hit);

	UFUNCTION(BlueprintImplementableEvent, Category="Parcel")
	void OnDamageStateChanged(EEGT_ParcelDamageState NewState, EEGT_ParcelDamageState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category="Parcel")
	void OnParcelSnapped(UEGT_SnapSpotComponent* Spot);

	
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
