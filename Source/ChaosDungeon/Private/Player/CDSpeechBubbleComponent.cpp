#include "Player/CDSpeechBubbleComponent.h"
#include "UI/CDSpeechBubbleWidget.h"

UCDSpeechBubbleComponent::UCDSpeechBubbleComponent()
{
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);
	SetPivot(FVector2D(0.5f, 1.f));		// 꼬리 끝(아래 가운데)이 소켓 위치
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetRelativeLocation(FVector(0.f, 0.f, 40.f));
}

int32 UCDSpeechBubbleComponent::ShowSpeech(const FText& text)
{
	UCDSpeechBubbleWidget* bubble = GetBubble();
	if ((bubble == nullptr) || text.IsEmpty())
	{
		return INDEX_NONE;
	}

	currentSpeechId = ++lastSpeechId;
	bubble->ShowSpeech(text);
	return currentSpeechId;
}

void UCDSpeechBubbleComponent::HideSpeech(int32 speechId)
{
	if (currentSpeechId == INDEX_NONE)
	{
		return;
	}

	// 다른 대사로 이미 바뀌었으면 그대로 둔다
	if ((speechId != INDEX_NONE) && (speechId != currentSpeechId))
	{
		return;
	}

	currentSpeechId = INDEX_NONE;
	if (UCDSpeechBubbleWidget* bubble = GetBubble())
	{
		bubble->HideSpeech();
	}
}

UCDSpeechBubbleWidget* UCDSpeechBubbleComponent::GetBubble() const
{
	return Cast<UCDSpeechBubbleWidget>(GetUserWidgetObject());
}
