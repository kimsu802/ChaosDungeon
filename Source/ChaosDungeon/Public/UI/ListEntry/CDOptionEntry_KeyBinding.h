#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/InputChord.h"
#include "UI/ListEntry/CDOptionEntryBase.h"
#include "CDOptionEntry_KeyBinding.generated.h"

class UCDOptionData_KeyBinding;
class UInputKeySelector;

/** 키 설정 줄 (WBP_OptionEntry_Key). 키캡을 클릭 → 키 입력 → 다른 슬롯과 겹치면 맞바꿈 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDOptionEntry_KeyBinding : public UCDOptionEntryBase
{
	GENERATED_BODY()

protected:
	// UUserWidget::NativeOnInitialized()
	virtual void NativeOnInitialized() override;

	// UCDOptionEntryBase::RefreshValue()
	virtual void RefreshValue() override;

private:
	/** 키 입력 완료 */
	UFUNCTION()
	void HandleKeySelected(FInputChord selectedKey);

	/** 키 입력 대기 시작 / 끝 */
	UFUNCTION()
	void HandleSelectingChanged();

	/** 연결된 키 데이터 */
	UCDOptionData_KeyBinding* GetKeyData() const;

protected:
	/** 키캡 (클릭하면 다음 키 입력을 받음) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UInputKeySelector> keySelector;

	/** 키 입력 대기 중 안내 ("바꿀 키를 누르세요") */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> selectingHintText;
};
