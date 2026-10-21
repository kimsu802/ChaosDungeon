// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/CDHealthBarWidget.h"
#include "CDBossHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UWidgetAnimation;
class UMaterialInterface;

/**
 * 보스 HP 바. 베이스(지연 바 · 끝부분 글로우)에 줄(라인) HP 를 얹는다.
 * 위젯 BP 의 Viewmodels 에 두 개를 Global Viewmodel Collection 으로 추가하고 아래처럼 바인딩한다.
 *   BossHealth (UCDHealthVM) . GetHealthPercent   → SetTotalPercent
 *   Stage      (UCDStageVM)  . BossHpLineCount    → SetTotalLines
 *   Stage      (UCDStageVM)  . IsBossVisible      → SetBossVisible
 *   Stage      (UCDStageVM)  . BossName           → nameText.Text (BP 에서 바로)
 * 주의: 베이스의 SetTargetPercent 는 바인딩하지 말 것 (줄 단위 비율이라 전체 HP 를 넣으면 한 줄로 보임)
 */
UCLASS()
class CHAOSDUNGEON_API UCDBossHealthBarWidget : public UCDHealthBarWidget
{
	GENERATED_BODY()

public:
	UCDBossHealthBarWidget(const FObjectInitializer& objectInitializer);

	// MVVM 바인딩 대상: 보스 전체 HP 비율 (0~1)
	UFUNCTION(BlueprintCallable, Category = "BossHP")
	void SetTotalPercent(float newTotalPercent);

	// MVVM 바인딩 대상: 보스 HP 줄 수 (UCDBossData::hpLineCount)
	UFUNCTION(BlueprintCallable, Category = "BossHP")
	void SetTotalLines(int32 newTotalLines);

	// MVVM 바인딩 대상: 보스 HP 바 표시 여부 (등장 → 표시, 보스 사망 · 다음 스테이지 → 숨김)
	UFUNCTION(BlueprintCallable, Category = "BossHP")
	void SetBossVisible(bool bVisible);

protected:
	// UCDHealthBarWidget::NativeConstruct() ← UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

private:
	// 전체 비율과 줄 수로 현재 줄과 줄 안의 비율을 계산해 반영
	void ApplyLines();

	// 현재 줄 / 다음 줄 색 교체 (남은 줄 1개면 다음 줄 바 숨김)
	void ApplyLineColors(int32 remainingLines);

	// 바의 fill 브러시를 머티리얼로 교체 (같은 머티리얼이면 건너뜀)
	static void SetBarFillMaterial(UProgressBar* bar, UMaterialInterface* material);

	// 줄 수 배지 텍스트 갱신
	void UpdateLineText(int32 remainingLines);

	// 전체 HP 퍼센트 텍스트 갱신 (소수점 한 자리)
	void UpdatePercentText();

protected:
	// HP 바 뒤에 깔리는 "다음 줄" 바 (Percent 항상 1)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> nextLineBar;

	// 남은 줄 수 "×45"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> lineText;

	// 전체 HP 퍼센트 "62.0%"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> percentText;

	// 보스 바 등장/퇴장 (없으면 Visibility 로 즉시 전환)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> showAnim;

	// 줄이 넘어갈 때 배지 팝
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> lineBadgeAnim;

	// 줄별 fill 머티리얼. (순서 규칙은 lineColors 와 같음)
	UPROPERTY(EditAnywhere, Category = "BossHP|Lines")
	TArray<TObjectPtr<UMaterialInterface>> lineMaterials;

private:
	// VM 에서 받은 전체 HP 비율
	float totalPercent = 1.f;

	// 총 줄 수
	int32 totalLines = 1;

	// 화면에 반영된 남은 줄 수 (INDEX_NONE = 아직 반영 안 됨)
	int32 shownLines = INDEX_NONE;

	// 현재 표시 상태 (같은 값이 다시 와도 애니메이션을 재생하지 않도록)
	bool bShown = false;
};
