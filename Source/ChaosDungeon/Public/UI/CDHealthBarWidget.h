// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CDHealthBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDHealthBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// MVVM 뷰 바인딩 대상. ViewModel의 HP 비율(0~1)이 바뀔 때 호출됨
	UFUNCTION(BlueprintCallable, Category = "HP")
	void SetTargetPercent(float newPercent);

protected:
	// UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

	// UUserWidget::NativeTick()
	virtual void NativeTick(const FGeometry& myGeometry, float inDeltaTime) override;

private:
	// 지연 바와 끝부분 글로우를 현재 값으로 갱신
	void RefreshVisuals();

	// 끝부분 글로우의 위치와 표시 여부 갱신
	void UpdateEdgeGlow();

protected:
	// 즉시 갱신되는 앞쪽 HP 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<class UProgressBar> hpBar;

	// 뒤에서 천천히 따라오는 지연 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> delayedBar;

	// 바 끝부분 글로우 (없어도 동작)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UImage> edgeGlow;

	// 피격 후 지연 바가 움직이기 시작할 때까지의 대기 시간(초)
	UPROPERTY(EditAnywhere, Category = "HP|Delay")
	float holdTime = 0.4f;

	// 지연 바가 줄어드는 속도(초당 비율)
	UPROPERTY(EditAnywhere, Category = "HP|Delay")
	float drainSpeed = 0.6f;

	// fill 텍스처의 원본 폭(px). 글로우 위치 계산용
	UPROPERTY(EditAnywhere, Category = "HP|Glow")
	float fillWidth = 734.f;

private:
	// 현재 목표 HP 비율
	float targetPercent = 1.f;

	// 지연 바가 현재 표시 중인 비율
	float delayedPercent = 1.f;

	// 지연 바가 움직이기 시작하기까지 남은 시간
	float holdRemaining = 0.f;

	// Glow 이미지 위치 오프셋
	UPROPERTY(EditAnywhere)
	FVector2D glowOffset;
};
