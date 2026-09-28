#include "UI/CDActivatableScreen.h"
#include "Kismet/GameplayStatics.h"

TOptional<FUIInputConfig> UCDActivatableScreen::GetDesiredInputConfig() const
{
	switch (inputMode)
	{
	case ECDScreenInputMode::Game:
	{
		// 쿼터뷰라 커서를 항상 보이게: 캡처 중에도 숨기지 않음
		return FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CaptureDuringMouseDown, false);
	}
	case ECDScreenInputMode::All:
	{
		return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::CaptureDuringMouseDown, false);
	}
	default:
	{
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	}
	}
}

void UCDActivatableScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	if (bPauseGame)
	{
		UGameplayStatics::SetGamePaused(this, true);
	}
}

void UCDActivatableScreen::NativeOnDeactivated()
{
	if (bPauseGame)
	{
		UGameplayStatics::SetGamePaused(this, false);
	}
	Super::NativeOnDeactivated();
}
