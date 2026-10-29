#include "UI/CDAwakenGaugeWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UCDAwakenGaugeWidget::SetTargetPercent(float newPercent)
{
	newPercent = FMath::Clamp(newPercent, 0.f, 1.f);

	// 첫 값 (HUD 생성 직후, 폰 교체 직후): 연출 없이 맞춘다
	if (bInitialized == false)
	{
		bInitialized = true;
		targetPercent = newPercent;
		displayPercent = newPercent;
		holdRemaining = 0.f;
		RefreshVisuals();
		TryPlayReady();
		return;
	}

	if (newPercent > targetPercent)
	{
		// 획득: 선행 바가 먼저 튀고, 본 바는 잠깐 멈췄다가 따라감 (연타 시 대기 시간 리셋)
		holdRemaining = gainHoldTime;
		PlayIfValid(gainAnim);
	}
	else if (newPercent < targetPercent)
	{
		// 소모: 대기 없이 바로 비워짐
		holdRemaining = 0.f;
	}

	targetPercent = newPercent;
	RefreshVisuals();
}

void UCDAwakenGaugeWidget::SetReady(bool bNewReady)
{
	if (bReady == bNewReady)
	{
		return;
	}

	bReady = bNewReady;
	if (bReady)
	{
		// 바가 아직 차오르는 중이면 Tick 에서 가득 찬 순간 재생
		TryPlayReady();
	}
	else
	{
		StopReady();
	}
}

void UCDAwakenGaugeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshVisuals();

	// 다시 Construct 되면 반복 연출이 멈춰 있으므로 준비 상태라면 다시 재생
	bReadyShown = false;
	TryPlayReady();
}

void UCDAwakenGaugeWidget::NativeTick(const FGeometry& myGeometry, float inDeltaTime)
{
	Super::NativeTick(myGeometry, inDeltaTime);

	// 해상도·창 크기가 바뀌면 바 폭이 달라지므로 글로우 위치를 다시 계산
	const float barWidth = fillBar->GetCachedGeometry().GetLocalSize().X;
	if (FMath::IsNearlyEqual(barWidth, lastBarWidth) == false)
	{
		lastBarWidth = barWidth;
		UpdateFillTip();
	}

	if (FMath::IsNearlyEqual(displayPercent, targetPercent))
	{
		return;
	}

	if (holdRemaining > 0.f)
	{
		holdRemaining -= inDeltaTime;
		return;
	}

	const float speed = (displayPercent < targetPercent) ? fillSpeed : drainSpeed;
	displayPercent = FMath::FInterpConstantTo(displayPercent, targetPercent, inDeltaTime, speed);
	RefreshVisuals();

	if (FMath::IsNearlyEqual(displayPercent, targetPercent))
	{
		TryPlayReady();
	}
}

void UCDAwakenGaugeWidget::RefreshVisuals()
{
	fillBar->SetPercent(displayPercent);

	if (gainBar)
	{
		// 소모 중에는 target < display 라 fillBar 뒤에 가려진다
		gainBar->SetPercent(targetPercent);
	}

	if (percentText)
	{
		// 반올림하면 99.5% 에서 100% 로 보이므로 내림
		const int32 percent = FMath::FloorToInt((displayPercent * 100.f) + KINDA_SMALL_NUMBER);
		percentText->SetText(FText::Format(INVTEXT("{0}%"), FText::AsNumber(percent)));
	}

	UpdateFillTip();
}

void UCDAwakenGaugeWidget::UpdateFillTip()
{
	if (fillTip == nullptr)
	{
		return;
	}

	const bool bShouldShow = (displayPercent > 0.f) && (displayPercent < 1.f);
	fillTip->SetRenderOpacity(bShouldShow ? 1.f : 0.f);

	if (bShouldShow == false)
	{
		return;
	}

	const float barWidth = fillBar->GetCachedGeometry().GetLocalSize().X;
	if (barWidth <= 0.f)
	{
		return;
	}

	// ProgressBar 의 Border Padding 만큼 양쪽이 줄어든 영역에 fill 이 그려진다
	const FVector2D borderPadding = fillBar->GetBorderPadding();
	const float fillAreaWidth = barWidth - (borderPadding.X * 2.f);

	// fill 텍스처의 실제 색 끝을 넘지 않도록
	const float fillRight = borderPadding.X + (fillAreaWidth * displayPercent);
	const float fillEnd = borderPadding.X + (fillAreaWidth * fillEndRatio);

	float tipWidth = fillTip->GetCachedGeometry().GetLocalSize().X;
	if (tipWidth <= 0.f)
	{
		tipWidth = fillTip->GetBrush().ImageSize.X;
	}

	const float x = FMath::Min(fillRight, fillEnd) - (tipWidth * 0.5f) + tipOffset.X;
	fillTip->SetRenderTranslation(FVector2D(x, tipOffset.Y));
}

void UCDAwakenGaugeWidget::TryPlayReady()
{
	// 바가 눈으로 가득 찬 뒤에 연출 (VM 은 이미 가득이어도 fillBar 는 따라가는 중일 수 있음)
	const bool bVisuallyFull = (displayPercent >= 1.f - KINDA_SMALL_NUMBER);
	if ((bReady == false) || bReadyShown || (bVisuallyFull == false))
	{
		return;
	}

	bReadyShown = true;
	PlayIfValid(readyAnim);
	// 0 = 무한 반복
	PlayIfValid(readyLoopAnim, 0);
}

void UCDAwakenGaugeWidget::StopReady()
{
	if (bReadyShown == false)
	{
		return;
	}

	bReadyShown = false;
	if (readyLoopAnim)
	{
		StopAnimation(readyLoopAnim);
	}

	// 점등 요소 끄기 + 소모 플래시 (준비 해제 = 각성기 사용)
	PlayIfValid(consumeAnim);
}

void UCDAwakenGaugeWidget::PlayIfValid(UWidgetAnimation* animation, int32 loopCount)
{
	if (animation)
	{
		PlayAnimation(animation, 0.f, loopCount);
	}
}
