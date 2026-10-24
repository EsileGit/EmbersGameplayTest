// Elise Rosati


#include "UI/EGT_InspectWidget.h"

#include "Component/EGT_InspectComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Hidden until inspect mode starts
	SetVisibility(ESlateVisibility::Collapsed);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::HandlePossessedPawnChanged);
		BindToPawn(PlayerController->GetPawn());
	}
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectWidget::NativeDestruct()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandlePossessedPawnChanged);
	}

	UnbindFromInspectComponent();

	Super::NativeDestruct();
}

#pragma region Binding

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectWidget::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	BindToPawn(NewPawn);
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectWidget::BindToPawn(APawn* Pawn)
{
	UnbindFromInspectComponent();

	if (!IsValid(Pawn))
	{
		return;
	}

	UEGT_InspectComponent* InspectComponent = Pawn->FindComponentByClass<UEGT_InspectComponent>();
	if (!InspectComponent)
	{
		return;
	}

	BoundInspectComponent = InspectComponent;
	InspectComponent->OnInspectModeChanged.AddUniqueDynamic(this, &ThisClass::HandleInspectModeChanged);

	// Push the current state so the Blueprint starts in sync
	HandleInspectModeChanged(InspectComponent->IsInspecting(), InspectComponent->GetInspectedActor());
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectWidget::UnbindFromInspectComponent()
{
	if (UEGT_InspectComponent* InspectComponent = BoundInspectComponent.Get())
	{
		InspectComponent->OnInspectModeChanged.RemoveDynamic(this, &ThisClass::HandleInspectModeChanged);
	}

	BoundInspectComponent.Reset();
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_InspectWidget::HandleInspectModeChanged(bool bIsInspecting, AActor* InspectedActor)
{
	if (!bIsInspecting)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		OnInspectEnded();
		return;
	}

	FEGT_ParcelInspectInfo Info;
	if (const AEGT_ParcelBase* Parcel = Cast<AEGT_ParcelBase>(InspectedActor))
	{
		Info = Parcel->GetInspectInfo();
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);
	OnInspectStarted(InspectedActor, Info);
}

#pragma endregion
