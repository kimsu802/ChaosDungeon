#pragma once

#include "UI/CDActivatableScreen.h"
#include "CDBootScreen.generated.h"

class UWidgetAnimation;

DECLARE_MULTICAST_DELEGATE(FCDBootFinishedSignature);

/**
 * 실행 직후 한 번 나오는 부트 화면 (WBP_BootScreen): ① 플레이 주의 안내 → ② 제작 안내 · 팀 로고.
 * - 연출은 위젯 애니메이션 sequenceAnim 하나에 전부 넣는다 (C++ 은 재생 · 건너뛰기 · 종료만)
 * - 아무 키 / 클릭: ① 중이면 ② 시작 지점으로, ② 중이면 바로 종료
 * - 끝나면 OnBootFinished 를 알리고 스스로 비활성화 → UIManager 가 타이틀을 띄운다
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDBootScreen : public UCDActivatableScreen
{
	GENERATED_BODY()

public:
	/** 포커스를 받아야 키 입력으로 건너뛸 수 있다 */
	UCDBootScreen(const FObjectInitializer& objectInitializer);

	/** 부트 화면이 끝났을 때 (정상 종료 · 건너뛰기 모두) */
	FCDBootFinishedSignature& OnBootFinished()
	{
		return bootFinished;
	}

protected:
	// UCommonActivatableWidget::NativeOnActivated()
	virtual void NativeOnActivated() override;

	// UCommonActivatableWidget::NativeGetDesiredFocusTarget()
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	// UUserWidget::NativeOnKeyDown()
	virtual FReply NativeOnKeyDown(const FGeometry& inGeometry, const FKeyEvent& inKeyEvent) override;

	// UUserWidget::NativeOnMouseButtonDown()
	virtual FReply NativeOnMouseButtonDown(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent) override;

	// UUserWidget::OnAnimationFinished_Implementation()
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* animation) override;

private:
	/** 건너뛰기: ① → ② 시작, ② → 종료 */
	void HandleSkip();

	/** 종료 알림 + 비활성화 (한 번만) */
	void Finish();

protected:
	/** 전체 연출 (① + ②). 이름이 정확히 sequenceAnim 이어야 한다 */
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> sequenceAnim;

	/** ② 장면이 시작되는 시간 (sequenceAnim 기준 초) */
	UPROPERTY(EditDefaultsOnly, Category = "Boot")
	float secondSceneTime = 4.6f;

	/** 이 시간 전에는 건너뛰기 무시 (실수로 누른 키 방지) */
	UPROPERTY(EditDefaultsOnly, Category = "Boot", meta = (ClampMin = 0))
	float minSkipTime = 0.8f;

	/** 건너뛰기 허용 */
	UPROPERTY(EditDefaultsOnly, Category = "Boot")
	bool bAllowSkip = true;

private:
	/** 종료 알림 */
	FCDBootFinishedSignature bootFinished;

	/** 이미 끝났는가 */
	bool bFinished = false;
};
