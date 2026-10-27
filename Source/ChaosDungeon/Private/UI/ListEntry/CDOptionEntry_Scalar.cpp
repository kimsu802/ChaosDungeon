#include "UI/ListEntry/CDOptionEntry_Scalar.h"
#include "CommonTextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "UI/ListData/CDOptionData_Scalar.h"

void UCDOptionEntry_Scalar::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	valueSlider->SetMinValue(0.f);
	valueSlider->SetMaxValue(1.f);
	valueSlider->OnValueChanged.AddDynamic(this, &ThisClass::HandleSliderChanged);
}

void UCDOptionEntry_Scalar::OnOptionDataSet(UCDOptionData* optionData)
{
	if (const UCDOptionData_Scalar* scalarData = Cast<UCDOptionData_Scalar>(optionData))
	{
		valueSlider->SetStepSize(scalarData->GetNormalizedStep());
	}
}

void UCDOptionEntry_Scalar::RefreshValue()
{
	if (const UCDOptionData_Scalar* scalarData = GetScalarData())
	{
		// SetValue 는 OnValueChanged 를 부르지 않으므로 되먹임 없음
		const float normalized = scalarData->GetNormalizedValue();
		valueSlider->SetValue(normalized);
		valueText->SetText(scalarData->GetValueText());

		if (valueFill)
		{
			valueFill->SetPercent(normalized);
		}
	}
}

void UCDOptionEntry_Scalar::HandleSliderChanged(float normalizedValue)
{
	if (UCDOptionData_Scalar* scalarData = GetScalarData())
	{
		scalarData->SetNormalizedValue(normalizedValue);
	}
}

UCDOptionData_Scalar* UCDOptionEntry_Scalar::GetScalarData() const
{
	return Cast<UCDOptionData_Scalar>(GetOptionData());
}
