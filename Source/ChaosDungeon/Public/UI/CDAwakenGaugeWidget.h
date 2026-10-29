#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CDAwakenGaugeWidget.generated.h"

class UProgressBar;
class UImage;
class UTextBlock;
class UWidgetAnimation;

/**
 * 각성 게이지 View. 표시 값은 MVVM 바인딩(BP), C++ 은 채움 연출(선행 바 → 본 바 추격)과 준비 연출 타이밍을 담당.
 * - 위젯 BP 의 Viewmodel 창에서 UCDAwakenGaugeVM 을 Creation Type = Global Viewmodel Collection, 이름 AwakenGauge 로 추가
 * - MVVM 바인딩: VM.GaugePercent → SetTargetPercent, VM.IsReady → SetReady, VM.KeyText → 키 텍스트
 * - 획득: gainBar 가 먼저 목표까지 튀어오르고, gainHoldTime 뒤 fillBar 가 따라 차오른다
 * - 소모: fillBar 가 drainSpeed 로 비워진다
 * - 준비: fillBar 가 실제로 가득 찬 순간 readyAnim(1회, 점등) + readyLoopAnim(반복, 맥동) 재생
 * - 준비 해제(각성기 사용): readyLoopAnim 정지 + consumeAnim(소모 플래시, 점등 요소 끄기)
 * - NativeTick / 위젯 애니메이션은 실시간 기준이라 각성기 TimeDilation 중에도 속도가 변하지 않는다
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDAwakenGaugeWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** MVVM 바인딩 대상: 게이지 비율(0~1)이 바뀔 때 호출됨 */
	UFUNCTION(BlueprintCallable, Category = "Awaken")
	void SetTargetPercent(float newPercent);

	/** MVVM 바인딩 대상: 각성기 사용 가능 여부가 바뀔 때 호출됨 */
	UFUNCTION(BlueprintCallable, Category = "Awaken")
	void SetReady(bool bNewReady);

protected:
	// UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

	// UUserWidget::NativeTick()
	virtual void NativeTick(const FGeometry& myGeometry, float inDeltaTime) override;

private:
	/** 바 · 텍스트 · 끝부분 글로우를 현재 표시 값으로 갱신 */
	void RefreshVisuals();

	/** 끝부분 글로우의 위치와 표시 여부 갱신 */
	void UpdateFillTip();

	/** 준비 상태이고 바가 가득 찼으면 준비 연출 재생 (1회) */
	void TryPlayReady();

	/** 준비 연출 정지 */
	void StopReady();

	/** 애니메이션이 있으면 처음부터 재생 */
	void PlayIfValid(UWidgetAnimation* animation, int32 loopCount = 1);

protected:
	/** 실제 게이지 (천천히 따라 차오름) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> fillBar;

	/** 획득 즉시 목표까지 먼저 차는 선행 바 (fillBar 아래에 배치) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> gainBar;

	/** fillBar 끝부분 글로우 (없어도 동작) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> fillTip;

	/** 퍼센트 텍스트 (표시 값 기준으로 숫자가 올라감, 없어도 동작) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> percentText;

	/** 획득할 때마다 짧게 번쩍임 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> gainAnim;

	/** 가득 찬 순간 1회 (버스트 + 엠블럼 점등). 끝 상태(점등)를 유지한다 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> readyAnim;

	/** 준비 상태 동안 반복 (맥동). readyAnim 과 다른 위젯/속성을 키로 잡을 것 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> readyLoopAnim;

	/** 준비 해제(각성기 사용) 시 1회: 소모 플래시 + readyAnim 이 켠 요소를 모두 끈다 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> consumeAnim;

	/** 획득 후 fillBar 가 따라가기 시작할 때까지의 대기 시간(초) */
	UPROPERTY(EditAnywhere, Category = "Awaken|Fill")
	float gainHoldTime = 0.12f;

	/** fillBar 가 차오르는 속도(초당 비율) */
	UPROPERTY(EditAnywhere, Category = "Awaken|Fill")
	float fillSpeed = 1.2f;

	/** 소모 시 비워지는 속도(초당 비율) */
	UPROPERTY(EditAnywhere, Category = "Awaken|Fill")
	float drainSpeed = 1.6f;

	/** fill 텍스처에서 실제 색이 끝나는 지점의 비율 (T_Awaken_Fill 은 끝까지 색이 차 있으므로 1) */
	UPROPERTY(EditAnywhere, Category = "Awaken|Tip", meta = (ClampMin = 0, ClampMax = 1))
	float fillEndRatio = 1.f;

	/** 끝부분 글로우 위치 미세 조정 */
	UPROPERTY(EditAnywhere, Category = "Awaken|Tip")
	FVector2D tipOffset = FVector2D::ZeroVector;

private:
	/** 목표 비율 (VM 값) */
	float targetPercent = 0.f;

	/** fillBar 가 현재 표시 중인 비율 */
	float displayPercent = 0.f;

	/** fillBar 가 따라가기 시작하기까지 남은 시간 */
	float holdRemaining = 0.f;

	/** 마지막으로 글로우 위치를 계산할 때의 바 폭 (해상도·창 크기 변경 감지용) */
	float lastBarWidth = 0.f;

	/** 첫 값을 받았는가 (첫 값은 연출 없이 바로 맞춘다) */
	bool bInitialized = false;

	/** VM 기준 준비 상태 */
	bool bReady = false;

	/** 준비 연출을 이미 재생했는가 */
	bool bReadyShown = false;
};
