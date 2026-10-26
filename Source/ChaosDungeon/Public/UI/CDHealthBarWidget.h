// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CDHealthBarWidget.generated.h"

class UProgressBar;
class UImage;

/**
 * HP 바 공용 베이스. 즉시 갱신되는 HP 바 + 뒤에서 따라오는 지연 바 + 끝부분 글로우.
 * - 플레이어 HP: 이 클래스를 그대로 부모로 사용
 * - 보스 HP: UCDBossHealthBarWidget 이 상속해 줄(라인) 계산만 추가
 */
UCLASS()
class CHAOSDUNGEON_API UCDHealthBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// MVVM 뷰 바인딩 대상. HP 바에 표시할 비율(0~1)이 바뀔 때 호출됨
	UFUNCTION(BlueprintCallable, Category = "HP")
	void SetTargetPercent(float newPercent);

	UFUNCTION(BlueprintCallable, Category = "HP")
	void SnapToPercent(float newPercent);

	// 현재 HP 바가 목표로 하는 비율
	FORCEINLINE float GetTargetPercent() const { return targetPercent; }

protected:
	// UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

	// UUserWidget::NativeTick()
	virtual void NativeTick(const FGeometry& myGeometry, float inDeltaTime) override;

	// 지연 바와 끝부분 글로우를 현재 값으로 갱신
	void RefreshVisuals();

	// 끝부분 글로우의 위치와 표시 여부 갱신
	void UpdateEdgeGlow();

	// 지연 바를 startPercent 위치에서 다시 시작 (holdTime 대기 후 현재 HP 까지 줄어듦)
	void RestartDelayedFrom(float startPercent);

protected:
	// 즉시 갱신되는 앞쪽 HP 바
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> hpBar;

	// 뒤에서 천천히 따라오는 지연 바
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> delayedBar;

	// 바 끝부분 글로우 (없어도 동작)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> edgeGlow;

	// 피격 후 지연 바가 움직이기 시작할 때까지의 대기 시간(초)
	UPROPERTY(EditAnywhere, Category = "HP|Delay")
	float holdTime = 0.4f;

	// 지연 바가 줄어드는 속도(초당 비율)
	UPROPERTY(EditAnywhere, Category = "HP|Delay")
	float drainSpeed = 0.6f;

	// fill 텍스처에서 실제 색이 끝나는 지점의 비율 (플레이어 728/734, 보스 1137/1143)
	UPROPERTY(EditAnywhere, Category = "HP|Glow", meta = (ClampMin = 0, ClampMax = 1))
	float fillEndRatio = 728.f / 734.f;

	// 글로우 이미지 위치 미세 조정
	UPROPERTY(EditAnywhere, Category = "HP|Glow")
	FVector2D glowOffset = FVector2D::ZeroVector;

private:
	// 현재 목표 HP 비율
	float targetPercent = 1.f;

	// 지연 바가 현재 표시 중인 비율
	float delayedPercent = 1.f;

	// 지연 바가 움직이기 시작하기까지 남은 시간
	float holdRemaining = 0.f;

	// 마지막으로 글로우 위치를 계산할 때의 바 폭 (해상도·창 크기 변경 감지용)
	float lastBarWidth = 0.f;
};
