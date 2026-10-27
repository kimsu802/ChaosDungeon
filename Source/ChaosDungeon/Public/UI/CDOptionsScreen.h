#pragma once

#include "CoreMinimal.h"
#include "UI/CDActivatableScreen.h"
#include "CDOptionsScreen.generated.h"

class UCDCommonTabListWidgetBase;
class UCDOptionData;
class UCDOptionListView;
class UCDOptionRegistry;
class UCommonButtonBase;
class UCommonTextBlock;
class UWidgetAnimation;

/**
 * 옵션 화면 (WBP_Options). screenSet.optionsScreen 에 지정 → 일시정지 메뉴의 "옵션" 버튼이 연다.
 * - 처음 열릴 때 UCDOptionRegistry 가 탭 · 항목을 만들고, 탭을 고르면 그 탭 항목을 리스트에 넣는다
 * - 값은 바꾸는 즉시 적용, 화면을 닫을 때 디스크에 저장 (GameUserSettings.ini + 키 매핑은 즉시 저장)
 * - 줄에 마우스를 올리면 오른쪽에 설명
 * - Esc = 닫기 (Class Defaults: Is Back Handler ✔). 일시정지 메뉴와 같은 스택이라 Pause Game ✔ 필요
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDOptionsScreen : public UCDActivatableScreen
{
	GENERATED_BODY()

protected:
	// UUserWidget::NativeOnInitialized()
	virtual void NativeOnInitialized() override;

	// UCDActivatableScreen::NativeOnActivated()
	virtual void NativeOnActivated() override;

	// UCDActivatableScreen::NativeOnDeactivated()
	virtual void NativeOnDeactivated() override;

	// UCommonActivatableWidget::NativeGetDesiredFocusTarget()
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	/** 탭 선택 → 리스트 교체 */
	UFUNCTION()
	void HandleTabSelected(FName tabId);

	/** 줄 호버 → 설명 표시 */
	void HandleItemHovered(UObject* item, bool bIsHovered);

	/** 기본값 버튼 → 확인창 → 현재 탭 초기화 */
	void HandleResetClicked();

	/** 닫기 버튼 */
	void HandleCloseClicked();

	/** 설명 칸 갱신 (nullptr 이면 안내 문구) */
	void ShowDescription(const UCDOptionData* option);

protected:
	/** 상단 탭 (그래픽 / 사운드 / 조작) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCDCommonTabListWidgetBase> tabList;

	/** 항목 리스트 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCDOptionListView> optionList;

	/** 설명 칸 제목 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> descTitleText;

	/** 설명 칸 본문 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> descBodyText;

	/** 현재 탭 기본값 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> resetButton;

	/** 닫기 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> closeButton;

	/** 열릴 때 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> openAnim;

private:
	/** 탭 · 항목 (처음 열릴 때 생성, 화면과 수명 같음) */
	UPROPERTY(Transient)
	TObjectPtr<UCDOptionRegistry> registry;

	/** 지금 보고 있는 탭 (다시 열면 같은 탭) */
	FName currentTabId;
};
