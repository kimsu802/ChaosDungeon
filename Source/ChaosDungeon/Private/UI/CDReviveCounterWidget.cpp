#include "UI/CDReviveCounterWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UCDReviveCounterWidget::SetInStage(bool bInStage)
{
	SetVisibility(bInStage ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	// 새 런이 시작되면 첫 값은 연출 없이 반영
	if (bInStage == false)
	{
		bHasValue = false;
	}
}

void UCDReviveCounterWidget::SetRemainingRevives(int32 remaining)
{
	const bool bDecreased = bHasValue && (shownRemaining >= 0) && (remaining >= 0) && (remaining < shownRemaining);

	shownRemaining = remaining;
	bHasValue = true;
	RefreshVisuals();

	if (bDecreased && loseAnim)
	{
		PlayAnimation(loseAnim);
	}
}

void UCDReviveCounterWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 바인딩이 오기 전엔 숨김 (Stage.IsInStage 가 true 면 곧바로 다시 켜짐)
	SetVisibility(ESlateVisibility::Collapsed);
	RefreshVisuals();
}

void UCDReviveCounterWidget::RefreshVisuals()
{
	const bool bUnlimited = (shownRemaining < 0);

	countText->SetVisibility(bUnlimited ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	if (infinityImage)
	{
		infinityImage->SetVisibility(bUnlimited ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	const bool bLastLife = (bUnlimited == false) && (shownRemaining == 0);
	const bool bLow = (bUnlimited == false) && (shownRemaining <= lowThreshold);

	if (bUnlimited == false)
	{
		countText->SetText(FText::AsNumber(shownRemaining));
		countText->SetColorAndOpacity(FSlateColor(bLastLife ? lastColor : (bLow ? lowColor : normalColor)));
	}

	if (iconImage)
	{
		iconImage->SetColorAndOpacity(bLastLife ? lastColor : FLinearColor::White);
	}

	// 마지막 기회: 경고 글로우 맥동을 무한 반복 / 아니면 멈추고 글로우를 끔
	if (bLastLife)
	{
		if (lastLifeAnim && (IsAnimationPlaying(lastLifeAnim) == false))
		{
			PlayAnimation(lastLifeAnim, 0.f, 0);
		}
		return;
	}

	if (lastLifeAnim && IsAnimationPlaying(lastLifeAnim))
	{
		StopAnimation(lastLifeAnim);
	}

	if (warningGlow)
	{
		warningGlow->SetRenderOpacity(0.f);
	}
}
