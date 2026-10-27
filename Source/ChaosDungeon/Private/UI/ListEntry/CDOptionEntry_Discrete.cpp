#include "UI/ListEntry/CDOptionEntry_Discrete.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "UI/ListData/CDOptionData_Discrete.h"

void UCDOptionEntry_Discrete::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	prevButton->OnClicked().AddUObject(this, &ThisClass::HandlePreviousClicked);
	nextButton->OnClicked().AddUObject(this, &ThisClass::HandleNextClicked);
}

void UCDOptionEntry_Discrete::RefreshValue()
{
	if (const UCDOptionData_Discrete* discreteData = GetDiscreteData())
	{
		valueText->SetText(discreteData->GetValueText());

		// 선택지가 1개 이하면 화살표 비활성 (해상도 목록을 못 얻은 경우 등)
		const bool bCanChange = (discreteData->GetOptionCount() > 1);
		prevButton->SetIsEnabled(bCanChange);
		nextButton->SetIsEnabled(bCanChange);
	}
}

void UCDOptionEntry_Discrete::HandlePreviousClicked()
{
	if (UCDOptionData_Discrete* discreteData = GetDiscreteData())
	{
		discreteData->SelectPrevious();
	}
}

void UCDOptionEntry_Discrete::HandleNextClicked()
{
	if (UCDOptionData_Discrete* discreteData = GetDiscreteData())
	{
		discreteData->SelectNext();
	}
}

UCDOptionData_Discrete* UCDOptionEntry_Discrete::GetDiscreteData() const
{
	return Cast<UCDOptionData_Discrete>(GetOptionData());
}
