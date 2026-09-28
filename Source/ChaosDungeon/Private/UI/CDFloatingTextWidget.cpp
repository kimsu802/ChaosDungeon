#include "UI/CDFloatingTextWidget.h"
#include "Kismet/GameplayStatics.h"

void UCDFloatingTextWidget::Play(const FVector& inWorldLocation, const FText& text, ECDFloatingTextStyle style)
{
	worldLocation = inWorldLocation;
	elapsed = 0.f;
	bInUse = true;
	SetVisibility(ESlateVisibility::HitTestInvisible);
	OnPlay(text, style);
}

void UCDFloatingTextWidget::NativeTick(const FGeometry& myGeometry, float inDeltaTime)
{
	Super::NativeTick(myGeometry, inDeltaTime);
	if (!bInUse)
	{
		return;
	}

	elapsed += inDeltaTime;
	if (elapsed >= lifetime)
	{
		Finish();
		return;
	}

	worldLocation.Z += riseSpeed * inDeltaTime;
	FVector2D screenPosition;
	if (UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), worldLocation, screenPosition))
	{
		SetPositionInViewport(screenPosition);
	}
}

void UCDFloatingTextWidget::Finish()
{
	bInUse = false;
	SetVisibility(ESlateVisibility::Collapsed);
}
