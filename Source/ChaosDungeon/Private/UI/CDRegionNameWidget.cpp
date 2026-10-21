// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CDRegionNameWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UCDRegionNameWidget::SetRegionName(FText newName)
{
	if (pendingName.IdenticalTo(newName))
	{
		return;
	}
	pendingName = newName;

	if (bShown)
	{
		// 보이는 중이면 사라졌다가 새 이름으로 등장
		SwapNameWithFade();
		return;
	}

	// 안 보이는 중이면 텍스트만 바꿔 두고, 보여야 하면 등장
	regionText->SetText(pendingName);
	RefreshShown();
}

void UCDRegionNameWidget::SetInStage(bool bNewInStage)
{
	bInStage = bNewInStage;
	RefreshShown();
}

void UCDRegionNameWidget::SetBossVisible(bool bNewBossVisible)
{
	bBossVisible = bNewBossVisible;
	RefreshShown();
}

void UCDRegionNameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (showAnim != nullptr)
	{
		FWidgetAnimationDynamicEvent finishedEvent;
		finishedEvent.BindDynamic(this, &ThisClass::HandleShowAnimFinished);
		BindToAnimationFinished(showAnim, finishedEvent);
	}

	// 바인딩 값이 오기 전에는 숨김
	if (bShown == false)
	{
		SetShownImmediate(false);
	}
}

void UCDRegionNameWidget::RefreshShown()
{
	const bool bShouldShow = ShouldShow();
	if (bShouldShow == bShown)
	{
		return;
	}
	bShown = bShouldShow;
	bSwapPending = false;

	if (showAnim == nullptr)
	{
		SetShownImmediate(bShown);
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (bShown)
	{
		regionText->SetText(pendingName);
		PlayAnimationForward(showAnim);
	}
	else
	{
		PlayAnimationReverse(showAnim, hideSpeed);
	}
}

void UCDRegionNameWidget::SwapNameWithFade()
{
	if (showAnim == nullptr)
	{
		regionText->SetText(pendingName);
		return;
	}

	// 퇴장 → (끝나면) 텍스트 교체 → 등장
	bSwapPending = true;
	PlayAnimationReverse(showAnim, hideSpeed);
}

void UCDRegionNameWidget::HandleShowAnimFinished()
{
	if (bSwapPending == false)
	{
		// 일반 퇴장이 끝났으면 레이아웃에서도 빼기
		if (bShown == false)
		{
			SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	bSwapPending = false;
	regionText->SetText(pendingName);

	if (ShouldShow())
	{
		PlayAnimationForward(showAnim);
	}
	else
	{
		bShown = false;
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCDRegionNameWidget::SetShownImmediate(bool bShow)
{
	SetVisibility(bShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
