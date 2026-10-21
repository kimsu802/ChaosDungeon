// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CDHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

void UCDHealthBarWidget::SetTargetPercent(float newPercent)
{
	newPercent = FMath::Clamp(newPercent, 0.f, 1.f);

	if (newPercent < targetPercent)
	{
		// 피격: 지연 바는 잠시 멈췄다가 따라감 (연타 시 대기 시간 리셋)
		holdRemaining = holdTime;
	}

	targetPercent = newPercent;

	if (targetPercent >= delayedPercent)
	{
		// 회복 (또는 보스 줄 전환으로 값이 올라감): 지연 바도 즉시 맞춤
		delayedPercent = targetPercent;
	}

	hpBar->SetPercent(targetPercent);
	RefreshVisuals();
}

void UCDHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	hpBar->SetPercent(targetPercent);
	RefreshVisuals();
}

void UCDHealthBarWidget::NativeTick(const FGeometry& myGeometry, float inDeltaTime)
{
	Super::NativeTick(myGeometry, inDeltaTime);

	// 해상도·창 크기가 바뀌면 바 폭이 달라지므로 글로우 위치를 다시 계산
	const float barWidth = hpBar->GetCachedGeometry().GetLocalSize().X;
	if (FMath::IsNearlyEqual(barWidth, lastBarWidth) == false)
	{
		lastBarWidth = barWidth;
		UpdateEdgeGlow();
	}

	// 따라갈 구간이 없으면 지연 바 처리는 건너뜀
	if (delayedPercent <= targetPercent)
	{
		return;
	}

	if (holdRemaining > 0.f)
	{
		holdRemaining -= inDeltaTime;
		return;
	}

	delayedPercent = FMath::FInterpConstantTo(delayedPercent, targetPercent, inDeltaTime, drainSpeed);
	RefreshVisuals();
}

void UCDHealthBarWidget::RefreshVisuals()
{
	delayedBar->SetPercent(delayedPercent);
	UpdateEdgeGlow();
}

void UCDHealthBarWidget::UpdateEdgeGlow()
{
	if (edgeGlow == nullptr)
	{
		return;
	}

	const bool shouldShow = (targetPercent > 0.f) && (targetPercent < 1.f);
	edgeGlow->SetRenderOpacity(shouldShow ? 1.f : 0.f);

	if (shouldShow == false)
	{
		return;
	}

	const float barWidth = hpBar->GetCachedGeometry().GetLocalSize().X;
	if (barWidth <= 0.f)
	{
		return;
	}

	// ProgressBar 의 Border Padding 만큼 양쪽이 줄어든 영역에 fill 이 그려진다
	const FVector2D borderPadding = hpBar->GetBorderPadding();
	const float fillAreaWidth = barWidth - (borderPadding.X * 2.f);

	// fill 텍스처의 실제 색 끝을 넘지 않도록
	const float fillRight = borderPadding.X + (fillAreaWidth * targetPercent);
	const float fillEnd = borderPadding.X + (fillAreaWidth * fillEndRatio);

	float glowWidth = edgeGlow->GetCachedGeometry().GetLocalSize().X;
	if (glowWidth <= 0.f)
	{
		glowWidth = edgeGlow->GetBrush().ImageSize.X;
	}

	const float x = FMath::Min(fillRight, fillEnd) - (glowWidth * 0.5f) + glowOffset.X;
	edgeGlow->SetRenderTranslation(FVector2D(x, glowOffset.Y));
}

void UCDHealthBarWidget::RestartDelayedFrom(float startPercent)
{
	delayedPercent = FMath::Clamp(startPercent, 0.f, 1.f);
	holdRemaining = holdTime;
}
