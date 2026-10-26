// Elise Rosati


#include "UI/EGT_CounterDeskWidget.h"

#include "Gameplay/EGT_CounterDesk.h"

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CounterDeskWidget::NativeDestruct()
{
	UnbindFromDesk();

	Super::NativeDestruct();
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CounterDeskWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const AEGT_CounterDesk* Desk = BoundDesk.Get();
	float RemainingTime = 0.0f;
	int32 CurrentPoints = 0;
	if (!Desk || !Desk->GetActiveRequestProgress(RemainingTime, CurrentPoints))
	{
		return;
	}

	const float RemainingTimeAlpha = ActiveRequestMaxTime > 0.0f ? RemainingTime / ActiveRequestMaxTime : 0.0f;
	OnRequestProgress(RemainingTime, RemainingTimeAlpha, CurrentPoints);
}

#pragma region Binding

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CounterDeskWidget::SetCounterDesk(AEGT_CounterDesk* Desk)
{
	UnbindFromDesk();

	if (!IsValid(Desk))
	{
		HandleActiveRequestChanged(false);
		return;
	}

	BoundDesk = Desk;
	Desk->OnActiveRequestChanged.AddUniqueDynamic(this, &ThisClass::HandleActiveRequestChanged);

	// Push the current state
	HandleActiveRequestChanged(Desk->HasActiveRequest());
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CounterDeskWidget::UnbindFromDesk()
{
	if (AEGT_CounterDesk* Desk = BoundDesk.Get())
	{
		Desk->OnActiveRequestChanged.RemoveDynamic(this, &ThisClass::HandleActiveRequestChanged);
	}

	BoundDesk.Reset();
}

// ---------------------------------------------------------------------------------------------------------------------
void UEGT_CounterDeskWidget::HandleActiveRequestChanged(bool bHasActiveRequest)
{
	FEGT_Request Request;
	const AEGT_CounterDesk* Desk = BoundDesk.Get();
	if (bHasActiveRequest && Desk && Desk->GetActiveRequest(Request))
	{
		ActiveRequestMaxTime = Request.MaxTime;
		OnRequestStarted(Request);
		return;
	}

	ActiveRequestMaxTime = 0.0f;
	OnRequestCleared();
}

#pragma endregion
