#pragma once

#include "CoreMinimal.h"
#include "UI/CDActivatableScreen.h"
#include "CDResultScreen.generated.h"

class UCommonButtonBase;
class UWidgetAnimation;

/**
 * 던전 결과 화면. Run.Finished 때 UIManager 가 Modal 레이어에 띄운다. (UCDUIScreenSet::resultScreen)
 * 표시 값은 Viewmodel(RunResult) 바인딩, 이 클래스는 성공/실패 연출 전환과 "돌아가기" 버튼만 담당한다.
 *   RunResult.record.bSuccess → SetSuccess
 *   RunResult.IsNewRecord     → SetNewRecord
 *   RunResult.Get...Text      → 각 TextBlock.Text (BP 에서 바로)
 * introAnim 이 끝나야 버튼 클릭이 처리된다. (보스 사망 연출이 끝나기 전 오입력 방지)
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDResultScreen : public UCDActivatableScreen
{
	GENERATED_BODY()

public:
	// MVVM 바인딩 대상: 성공이면 CLEAR, 실패면 FAILED 타이틀을 보인다
	UFUNCTION(BlueprintCallable, Category = "Result")
	void SetSuccess(bool bSuccess);

	// MVVM 바인딩 대상: 최고 기록 갱신 배지 표시
	UFUNCTION(BlueprintCallable, Category = "Result")
	void SetNewRecord(bool bNewRecord);

protected:
	// UUserWidget::NativeOnInitialized()
	virtual void NativeOnInitialized() override;

	// UCDActivatableScreen::NativeOnActivated() ← UCommonActivatableWidget::NativeOnActivated()
	virtual void NativeOnActivated() override;

	// UCommonActivatableWidget::NativeGetDesiredFocusTarget()
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	// 등장 연출 종료 → 버튼 활성화
	UFUNCTION()
	void HandleIntroFinished();

	// "입구로 돌아가기" 클릭
	void HandleReturnClicked();

protected:
	// 입구(허브 레벨)로 돌아가는 버튼
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> returnButton;

	// 성공 타이틀 묶음 (CLEAR 이미지 · 광선 등)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> clearTitle;

	// 실패 타이틀 묶음 (FAILED 이미지)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> failTitle;

	// 최고 기록 배지
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> newRecordBadge;

	// 전체 등장 연출 (암전 → 타이틀 → 패널 → 버튼)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> introAnim;

	// 최고 기록 배지 팝 (introAnim 이 끝난 뒤 재생)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> newRecordAnim;

private:
	// 최고 기록 갱신 여부 (introAnim 종료 후 배지 연출에 사용)
	bool bShowNewRecord = false;

	// introAnim 재생 중 (버튼 클릭 무시)
	bool bInputLocked = false;

	// 돌아가기 중복 클릭 방지
	bool bReturning = false;
};
