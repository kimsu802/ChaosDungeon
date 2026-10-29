#include "UI/CDAwakenOverlayWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDMessages.h"

void UCDAwakenOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 입력을 막지 않는다
	SetVisibility(ESlateVisibility::HitTestInvisible);

	UCDMessageSubsystem& messages = UCDMessageSubsystem::Get(this);
	messages.Unlisten(phaseHandle);
	phaseHandle = messages.Listen(CDTags::Msg_Awaken_Phase, this, &ThisClass::HandlePhase);
}

void UCDAwakenOverlayWidget::NativeDestruct()
{
	UCDMessageSubsystem::Get(this).Unlisten(phaseHandle);
	Super::NativeDestruct();
}

void UCDAwakenOverlayWidget::HandlePhase(const FCDAwakenPhaseMessage& message)
{
	// 다른 플레이어의 연출은 무시
	const AActor* instigator = message.instigator.Get();
	if (instigator && (instigator != GetOwningPlayerPawn()))
	{
		return;
	}

	switch (message.phase)
	{
	case ECDAwakenPhase::Charge:
		StopIfValid(recoverAnim);
		PlayIfValid(chargeAnim);
		// 0 = 무한 반복
		PlayIfValid(chargeLoopAnim, 0);
		break;

	case ECDAwakenPhase::Release:
		StopIfValid(chargeLoopAnim);
		if (speedLines)
		{
			speedLines->SetRenderTransformAngle(message.screenAngle);
		}
		PlayIfValid(releaseAnim);
		break;

	case ECDAwakenPhase::Freeze:
		PlayIfValid(freezeAnim);
		break;

	case ECDAwakenPhase::Impact:
		PlayIfValid(impactAnim);
		break;

	case ECDAwakenPhase::Recover:
		StopIfValid(chargeLoopAnim);
		PlayIfValid(recoverAnim);
		break;

	default:
		break;
	}

	onPhaseChanged.Broadcast(message.phase);
}

void UCDAwakenOverlayWidget::PlayIfValid(UWidgetAnimation* animation, int32 loopCount)
{
	if (animation)
	{
		PlayAnimation(animation, 0.f, loopCount);
	}
}

void UCDAwakenOverlayWidget::StopIfValid(UWidgetAnimation* animation)
{
	if (animation)
	{
		StopAnimation(animation);
	}
}
