#pragma once

#include "CoreMinimal.h"
#include "UI/ListEntry/CDOptionEntryBase.h"
#include "CDOptionEntry_Discrete.generated.h"

class UCDOptionData_Discrete;
class UCommonButtonBase;

/** 좌우 선택 줄 (WBP_OptionEntry_Discrete). "< 높음 >" */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDOptionEntry_Discrete : public UCDOptionEntryBase
{
	GENERATED_BODY()

protected:
	// UUserWidget::NativeOnInitialized()
	virtual void NativeOnInitialized() override;

	// UCDOptionEntryBase::RefreshValue()
	virtual void RefreshValue() override;

private:
	/** 이전 선택지 */
	void HandlePreviousClicked();

	/** 다음 선택지 */
	void HandleNextClicked();

	/** 연결된 선택 데이터 */
	UCDOptionData_Discrete* GetDiscreteData() const;

protected:
	/** 왼쪽 화살표 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> prevButton;

	/** 오른쪽 화살표 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> nextButton;

	/** 현재 선택지 글자 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> valueText;
};
