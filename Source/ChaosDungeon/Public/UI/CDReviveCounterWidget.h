#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CDReviveCounterWidget.generated.h"

class UImage;
class UTextBlock;
class UWidgetAnimation;

/**
 * 좌상단 진행도 아래 "남은 부활" 표시. 던전(스테이지)에서만 보이고 허브에서는 숨김.
 * 위젯 BP 의 Viewmodels 에 Stage (UCDStageVM, Global Viewmodel Collection) 를 추가하고 바인딩한다.
 *   Stage.IsInStage        → SetInStage
 *   Stage.RemainingDeaths  → SetRemainingRevives   (-1 = 무제한 → ∞ 표시)
 * 남은 횟수가 줄면 loseAnim, 0 (다음 사망 = 실패) 이면 lastLifeAnim 을 반복 재생한다.
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDReviveCounterWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// MVVM 바인딩 대상: 스테이지 안이면 표시, 허브면 숨김
	UFUNCTION(BlueprintCallable, Category = "Revive")
	void SetInStage(bool bInStage);

	// MVVM 바인딩 대상: 남은 부활 횟수 (-1 = 무제한)
	UFUNCTION(BlueprintCallable, Category = "Revive")
	void SetRemainingRevives(int32 remaining);

protected:
	// UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

private:
	// 남은 횟수에 맞춰 숫자 · 색 · 경고 연출 갱신
	void RefreshVisuals();

protected:
	// 남은 횟수 숫자
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> countText;

	// 무제한일 때 숫자 대신 보이는 ∞ 이미지
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> infinityImage;

	// 부활 아이콘 (마지막 기회일 때 붉게)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> iconImage;

	// 마지막 기회 경고 글로우 (lastLifeAnim 이 Render Opacity 를 움직임)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> warningGlow;

	// 부활 횟수가 줄어들 때 (숫자 팝 + 아이콘 번쩍)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> loseAnim;

	// 남은 횟수 0 일 때 반복 (경고 글로우 맥동)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> lastLifeAnim;

	// 여유 있을 때 숫자 색
	UPROPERTY(EditAnywhere, Category = "Revive")
	FLinearColor normalColor = FLinearColor::FromSRGBColor(FColor(0xF0, 0xE8, 0xD8));

	// 얼마 안 남았을 때 숫자 색 (lowThreshold 이하)
	UPROPERTY(EditAnywhere, Category = "Revive")
	FLinearColor lowColor = FLinearColor::FromSRGBColor(FColor(0xFF, 0xAA, 0x46));

	// 0 (다음 사망 = 실패) 일 때 숫자 · 아이콘 색
	UPROPERTY(EditAnywhere, Category = "Revive")
	FLinearColor lastColor = FLinearColor::FromSRGBColor(FColor(0xFF, 0x5A, 0x46));

	// 이 값 이하부터 lowColor
	UPROPERTY(EditAnywhere, Category = "Revive", meta = (ClampMin = 0))
	int32 lowThreshold = 1;

private:
	// 현재 표시 중인 남은 횟수 (-1 = 무제한)
	int32 shownRemaining = -1;

	// 아직 한 번도 값을 받지 않았는가 (첫 값에는 loseAnim 을 재생하지 않음)
	bool bHasValue = false;
};
