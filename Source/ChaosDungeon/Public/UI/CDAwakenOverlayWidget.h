#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Core/CDMessageSubsystem.h"
#include "Core/CDTypes.h"
#include "CDAwakenOverlayWidget.generated.h"

class UImage;
class UWidgetAnimation;
struct FCDAwakenPhaseMessage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCDAwakenPhaseChangedSignature, ECDAwakenPhase, phase);

/**
 * 각성기 연출의 화면 위 요소 (레터박스 · 플래시 · 집중선 · 속도선). WBP_Game 최상단에 전체 화면으로 배치.
 * - Msg.Awaken.Phase 를 구독해 단계별 위젯 애니메이션을 재생한다
 * - 위젯 애니메이션은 실시간이라 각성기 TimeDilation 의 영향을 받지 않는다
 * - HUD 숨김 등 다른 위젯의 반응은 onPhaseChanged 에 바인딩 (WBP_Game 에서)
 * - 모든 자식은 기본 RenderOpacity 0 / 레터박스는 화면 밖으로 이동한 상태로 두고, recoverAnim 이 그 상태로 되돌린다
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDAwakenOverlayWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** 단계가 바뀔 때 (HUD 페이드 등) */
	UPROPERTY(BlueprintAssignable, Category = "Awaken")
	FCDAwakenPhaseChangedSignature onPhaseChanged;

protected:
	// UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

	// UUserWidget::NativeDestruct()
	virtual void NativeDestruct() override;

private:
	/** Msg.Awaken.Phase 수신 */
	void HandlePhase(const FCDAwakenPhaseMessage& message);

	/** 애니메이션이 있으면 처음부터 재생 */
	void PlayIfValid(UWidgetAnimation* animation, int32 loopCount = 1);

	/** 애니메이션이 있으면 정지 */
	void StopIfValid(UWidgetAnimation* animation);

protected:
	/** 돌진 방향 속도선 (화면보다 크게 배치, 중심 기준 회전) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> speedLines;

	/** Charge 진입 1회: 플래시 · 레터박스 in · 집중선 페이드 인 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> chargeAnim;

	/** Charge 동안 반복: 집중선 A/B 깜빡임 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> chargeLoopAnim;

	/** Release 1회: 화이트 1프레임 · 속도선 · 집중선 끄기 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> releaseAnim;

	/** Freeze 1회: (선택) 화면 가장자리 어둡게 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> freezeAnim;

	/** Impact 1회: 플래시 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> impactAnim;

	/** Recover 1회: 레터박스 out, 모든 요소를 기본 상태로 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> recoverAnim;

private:
	/** Msg.Awaken.Phase 구독 핸들 */
	FCDListenerHandle phaseHandle;
};
