#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CDSpeechBubbleWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

/**
 * 머리 위 대사창 (WBP_SpeechBubble). 문장을 한 번에 다 보여준다.
 * 모양(색 · 폰트 · 글로우)은 위젯 BP 에서 정하고, C++ 은 텍스트 교체와 등장/퇴장만 담당.
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDSpeechBubbleWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** 대사를 띄운다. 이미 떠 있으면 바로 교체 */
	void ShowSpeech(const FText& text);

	/** 대사를 내린다 (hideAnim 이 있으면 재생 후 숨김) */
	void HideSpeech();

protected:
	// UUserWidget::NativeConstruct()
	virtual void NativeConstruct() override;

private:
	/** hideAnim 끝 → 숨김 */
	UFUNCTION()
	void HandleHideFinished();

protected:
	/** 대사 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> speechText;

	/** 등장 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> showAnim;

	/** 퇴장 */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> hideAnim;

private:
	/** 대사가 떠 있는가 (hideAnim 재생 중은 false) */
	bool bShowing = false;
};
