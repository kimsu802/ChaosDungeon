#pragma once

#include "CoreMinimal.h"
#include "UI/ListEntry/CDOptionEntryBase.h"
#include "CDOptionEntry_Scalar.generated.h"

class UCDOptionData_Scalar;
class UProgressBar;
class USlider;

/**
 * 슬라이더 줄 (WBP_OptionEntry_Scalar). 슬라이더는 0 ~ 1, 실제 범위 · step 은 데이터가 맞춘다.
 * Slider 에는 채움 부분이 없어서, 같은 자리 아래에 ProgressBar(valueFill) 를 깔아 트랙 + 금색 채움을 그린다.
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDOptionEntry_Scalar : public UCDOptionEntryBase
{
	GENERATED_BODY()

protected:
	// UUserWidget::NativeOnInitialized()
	virtual void NativeOnInitialized() override;

	// UCDOptionEntryBase::OnOptionDataSet()
	virtual void OnOptionDataSet(UCDOptionData* optionData) override;

	// UCDOptionEntryBase::RefreshValue()
	virtual void RefreshValue() override;

private:
	/** 슬라이더 이동 */
	UFUNCTION()
	void HandleSliderChanged(float normalizedValue);

	/** 연결된 슬라이더 데이터 */
	UCDOptionData_Scalar* GetScalarData() const;

protected:
	/** 값 슬라이더 (Min 0, Max 1) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> valueSlider;

	/** 슬라이더 아래 트랙 + 채움 (Slider 의 바 이미지는 투명) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> valueFill;

	/** 값 글자 ("80%") */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> valueText;
};
