// Elise Rosati


#include "UI/EGT_HUD.h"

#include "EmbersGameplayTest.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_HUD::BeginPlay()
{
	Super::BeginPlay();

	// Standard boilerplate, in a solo-player game, we could remove it, but this is quite harmless
	if (!PlayerOwner || !PlayerOwner->IsLocalPlayerController())
	{
		return;
	}

	if (!OverlayWidgetClass)
	{
		UE_LOG(LogEmbersGameplayTest, Warning, TEXT("'%s' has no HUDWidgetClass set: no HUD will be displayed."), *GetNameSafe(this));
		return;
	}

	OverlayWidget = CreateWidget<UUserWidget>(PlayerOwner, OverlayWidgetClass);
	if (OverlayWidget)
	{
		OverlayWidget->AddToPlayerScreen(0);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void AEGT_HUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OverlayWidget)
	{
		OverlayWidget->RemoveFromParent();
		OverlayWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
