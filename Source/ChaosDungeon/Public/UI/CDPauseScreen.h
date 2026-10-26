#pragma once

#include "CoreMinimal.h"
#include "UI/CDActivatableScreen.h"
#include "CDPauseScreen.generated.h"

class UCommonButtonBase;
class UWidgetAnimation;

/**
 * Esc 일시정지 메뉴. UCDUIManagerSubsystem::TogglePauseMenu 가 GameMenu 레이어에 띄운다. (UCDUIScreenSet::pauseScreen)
 * Class Defaults: Input Mode = Menu, Pause Game = true, Is Back Handler = true (Esc 로 닫힘)
 *   계속하기   → 닫기
 *   옵션       → UIManager::ShowOptions (screenSet.optionsScreen 이 비어 있으면 버튼 비활성)
 *   조작법     → UIManager::ToggleGuide
 *   던전 포기  → 확인창 → ACDDungeonGameMode::AbandonRun (던전에서만 보임) → FAILED 결과 화면
 *   메인 메뉴  → 확인창 → UCDRunSubsystem::ReturnToMainMenu (허브 레벨 + 타이틀)
 *   게임 종료  → 확인창 → QuitGame
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDPauseScreen : public UCDActivatableScreen
{
	GENERATED_BODY()

protected:
	// UUserWidget::NativeOnInitialized()
	virtual void NativeOnInitialized() override;

	// UCDActivatableScreen::NativeOnActivated() ← UCommonActivatableWidget::NativeOnActivated()
	virtual void NativeOnActivated() override;

	// UCommonActivatableWidget::NativeGetDesiredFocusTarget()
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	// 계속하기
	void HandleResumeClicked();

	// 옵션
	void HandleOptionsClicked();

	// 조작법
	void HandleGuideClicked();

	// 던전 포기 (확인 후)
	void HandleAbandonClicked();

	// 메인 메뉴 (확인 후)
	void HandleMainMenuClicked();

	// 게임 종료 (확인 후)
	void HandleQuitClicked();

	// 던전(스테이지) 안인가 — 던전 포기 버튼 표시 · 확인 문구 분기용
	bool IsInDungeon() const;

protected:
	// 계속하기 (기본 포커스)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> resumeButton;

	// 메인 메뉴로
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> mainMenuButton;

	// 옵션
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> optionsButton;

	// 조작법
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> guideButton;

	// 던전 포기 (허브에서는 숨김)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> abandonButton;

	// 게임 종료
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> quitButton;

	// 등장 연출 (배경 블러 · 패널 스케일 · 버튼 순차 등장)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> openAnim;
};
