// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CDBossHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Materials/MaterialInterface.h"

UCDBossHealthBarWidget::UCDBossHealthBarWidget(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	// 보스는 줄이 길어 지연 바를 조금 빠르게, 글로우 끝 비율은 보스 fill(1137/1143) 기준
	drainSpeed = 0.9f;
	fillEndRatio = 1137.f / 1143.f;

}

void UCDBossHealthBarWidget::SetTotalPercent(float newTotalPercent)
{
	totalPercent = FMath::Clamp(newTotalPercent, 0.f, 1.f);
	ApplyLines();
	UpdatePercentText();
}

void UCDBossHealthBarWidget::SetTotalLines(int32 newTotalLines)
{
	totalLines = FMath::Max(1, newTotalLines);

	// 새 보스: 줄 색 · 배지를 처음부터 다시 계산 (배지 팝 없이)
	shownLines = INDEX_NONE;
	ApplyLines();
}

void UCDBossHealthBarWidget::SetBossVisible(bool bVisible)
{
	if (bVisible == bShown)
	{
		return;
	}
	bShown = bVisible;

	if (showAnim == nullptr)
	{
		SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (bVisible)
	{
		PlayAnimationForward(showAnim);
	}
	else
	{
		PlayAnimationReverse(showAnim, 1.4f);
	}
}

void UCDBossHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	nextLineBar->SetPercent(1.f);

	// 바인딩이 오기 전엔 숨김 (Stage.IsBossVisible 이 true 면 곧바로 SetBossVisible 이 다시 켬)
	if (bShown == false)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	ApplyLines();
	UpdatePercentText();
}

void UCDBossHealthBarWidget::ApplyLines()
{
	// 전체 비율 → 남은 줄 수와 그 줄 안의 비율
	const float scaled = totalPercent * totalLines;
	const int32 remainingLines = FMath::Clamp(FMath::CeilToInt(scaled), 1, totalLines);
	const float linePercent = FMath::Clamp(scaled - (remainingLines - 1), 0.f, 1.f);

	if (remainingLines != shownLines)
	{
		const bool bLineDropped = (shownLines != INDEX_NONE) && (remainingLines < shownLines);

		shownLines = remainingLines;
		ApplyLineColors(remainingLines);
		UpdateLineText(remainingLines);

		if (bLineDropped)
		{
			// 줄을 넘긴 피격: 새 줄의 끝(1.0)에서부터 지연 바가 깎이도록 다시 시작
			// (안 하면 linePercent 가 커져 "회복"으로 처리되고 지연 연출이 사라짐)
			RestartDelayedFrom(1.f);

			if (lineBadgeAnim != nullptr)
			{
				PlayAnimation(lineBadgeAnim);
			}
		}
	}

	SetTargetPercent(linePercent);
}

void UCDBossHealthBarWidget::ApplyLineColors(int32 remainingLines)
{
	const bool bHasNextLine = (remainingLines > 1);

	// 마지막 줄: 뒤에 깔릴 바가 없음
	nextLineBar->SetRenderOpacity(bHasNextLine ? 1.f : 0.f);

	// 머티리얼 모드: 색은 머티리얼이 표현하므로 틴트는 흰색으로 두고 브러시만 교체
	if (lineMaterials.Num() > 0)
	{
		hpBar->SetFillColorAndOpacity(FLinearColor::White);
		SetBarFillMaterial(hpBar, lineMaterials[(remainingLines - 1) % lineMaterials.Num()]);

		if (bHasNextLine)
		{
			nextLineBar->SetFillColorAndOpacity(FLinearColor::White);
			SetBarFillMaterial(nextLineBar, lineMaterials[(remainingLines - 2) % lineMaterials.Num()]);
		}
		return;
	}
}

void UCDBossHealthBarWidget::SetBarFillMaterial(UProgressBar* bar, UMaterialInterface* material)
{
	if (material == nullptr)
	{
		return;
	}

	FProgressBarStyle style = bar->GetWidgetStyle();
	if (style.FillImage.GetResourceObject() == material)
	{
		return;
	}

	// Draw As · Image Size 등 나머지 브러시 설정은 디자이너 값을 유지
	style.FillImage.SetResourceObject(material);
	bar->SetWidgetStyle(style);
}

void UCDBossHealthBarWidget::UpdateLineText(int32 remainingLines)
{
	// × = 곱하기 기호(×). 소스 인코딩 문제를 피하려고 이스케이프로 씀
	lineText->SetText(FText::FromString(FString::Printf(TEXT("\u00D7%d"), remainingLines)));
}

void UCDBossHealthBarWidget::UpdatePercentText()
{
	if (percentText == nullptr)
	{
		return;
	}

	FNumberFormattingOptions options;
	options.MinimumFractionalDigits = 1;
	options.MaximumFractionalDigits = 1;
	percentText->SetText(FText::AsPercent(totalPercent, &options));
}