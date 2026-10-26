// Elise Rosati


#include "Item/EGT_Sticker.h"

#include "EmbersGameplayTest.h"
#include "Gameplay/EGT_GameplayLibrary.h"

#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"

// ---------------------------------------------------------------------------------------------------------------------
AEGT_Sticker::AEGT_Sticker()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(false);

	StickerDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("Sticker Decal"));
	SetRootComponent(StickerDecal);

	StickerDecal->DecalSize = FVector(2.0f, 8.0f, 8.0f);
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_Sticker::SetStickerColor(EEGT_StickerColor InColor)
{
	StickerColor = InColor;

	const FLinearColor LinearColor = UEGT_GameplayLibrary::GetStickerLinearColor(this, InColor);

	UMaterialInstanceDynamic* DynamicMaterial = StickerDecal->CreateDynamicMaterialInstance();
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(ColorParameterName, LinearColor);
	}
	else
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("[Sticker] '%s' has no decal material set"), *GetActorNameOrLabel());
	}

	OnStickerColorSet(InColor, LinearColor);
}
