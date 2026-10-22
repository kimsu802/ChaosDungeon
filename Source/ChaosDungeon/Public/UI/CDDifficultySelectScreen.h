#pragma once

#include "CoreMinimal.h"
#include "UI/CDActivatableScreen.h"
#include "Core/CDTypes.h"
#include "CDDifficultySelectScreen.generated.h"

class UCommonButtonBase;
class UCDDifficultyCard;
class UTextBlock;
class UWidgetAnimation;
class UCommonButtonGroupBase;

/**
 * 던전 입구 NPC/문(ACDInteractable, DungeonEntrance)이 Modal 레이어에 띄우는 난이도 선택 화면.
 * 카드 3장 중 하나를 고르고 "입장" → UCDRunSubsystem::StartRun(선택 난이도)
 * 취소 / Esc → 화면 닫기 (CommonUI Back 처리)
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDDifficultySelectScreen : public UCDActivatableScreen
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
	// 카드 선택 (나머지 카드는 선택 해제, 설명 갱신)
	void SelectDifficulty(ECDDifficulty newDifficulty);

	// 카드 클릭
	void HandleCardClicked(ECDDifficulty clickedDifficulty);

	// "입장" 클릭
	void HandleStartClicked();

	// "취소" 클릭
	void HandleCancelClicked();

	// 난이도 → 카드
	UCDDifficultyCard* FindCard(ECDDifficulty targetDifficulty) const;

protected:
	// 쉬움 카드
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCDDifficultyCard> easyCard;

	// 보통 카드
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCDDifficultyCard> normalCard;

	// 어려움 카드
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCDDifficultyCard> hardCard;

	// 입장 버튼
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> startButton;

	// 취소 버튼
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> cancelButton;

	// 선택한 난이도 설명
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> descriptionText;

	// 팝업 등장 연출 (Scale 0.95 → 1 + Fade, 선택)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> openAnim;

	// 화면을 열 때 기본 선택 난이도
	UPROPERTY(EditDefaultsOnly, Category = "Difficulty")
	ECDDifficulty defaultDifficulty = ECDDifficulty::Normal;

private:
	// 카드 3장 단일 선택 그룹 (하나를 고르면 나머지는 그룹이 선택 해제)
	UPROPERTY(Transient)
	TObjectPtr<UCommonButtonGroupBase> cardGroup;

	// 현재 선택된 난이도
	ECDDifficulty selectedDifficulty = ECDDifficulty::Normal;

	// 입장 중복 클릭 방지
	bool bStarting = false;
};
