#include "UI/CDSpeechBubbleWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"

void UCDSpeechBubbleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (hideAnim != nullptr)
	{
		FWidgetAnimationDynamicEvent finished;
		finished.BindDynamic(this, &ThisClass::HandleHideFinished);
		BindToAnimationFinished(hideAnim, finished);
	}

	// 말하기 전엔 숨김
	bShowing = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCDSpeechBubbleWidget::ShowSpeech(const FText& text)
{
	StopAllAnimations();
	speechText->SetText(text);

	bShowing = true;
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (showAnim != nullptr)
	{
		PlayAnimation(showAnim);
	}
}

void UCDSpeechBubbleWidget::HideSpeech()
{
	if (bShowing == false)
	{
		return;
	}

	bShowing = false;
	if (hideAnim == nullptr)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	PlayAnimation(hideAnim);
}

void UCDSpeechBubbleWidget::HandleHideFinished()
{
	// hideAnim 도중 새 대사가 시작됐으면 숨기지 않음 (StopAllAnimations 도 finished 를 부름)
	if (bShowing)
	{
		return;
	}

	SetVisibility(ESlateVisibility::Collapsed);
}
