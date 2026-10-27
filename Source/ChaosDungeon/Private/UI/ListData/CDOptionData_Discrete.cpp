#include "UI/ListData/CDOptionData_Discrete.h"

#define LOCTEXT_NAMESPACE "CDOptionData"

void UCDOptionData_Discrete::InitDiscrete(TArray<FText>&& inOptions, TFunction<int32()>&& inGetter, TFunction<void(int32)>&& inSetter, int32 inDefaultIndex)
{
	options = MoveTemp(inOptions);
	getter = MoveTemp(inGetter);
	setter = MoveTemp(inSetter);
	defaultIndex = inDefaultIndex;
	customText = LOCTEXT("Custom", "사용자 지정");
}

int32 UCDOptionData_Discrete::GetIndex() const
{
	const int32 index = getter ? getter() : defaultIndex;
	return options.IsValidIndex(index) ? index : INDEX_NONE;
}

void UCDOptionData_Discrete::SetIndex(int32 newIndex)
{
	if ((options.IsValidIndex(newIndex) == false) || (newIndex == GetIndex()))
	{
		return;
	}

	if (setter)
	{
		setter(newIndex);
	}
	NotifyValueChanged();
}

void UCDOptionData_Discrete::SelectNext()
{
	if (options.Num() == 0)
	{
		return;
	}

	// "사용자 지정" 상태에서는 첫 선택지부터
	const int32 current = GetIndex();
	SetIndex((current == INDEX_NONE) ? 0 : ((current + 1) % options.Num()));
}

void UCDOptionData_Discrete::SelectPrevious()
{
	if (options.Num() == 0)
	{
		return;
	}

	const int32 current = GetIndex();
	SetIndex((current == INDEX_NONE) ? (options.Num() - 1) : ((current - 1 + options.Num()) % options.Num()));
}

FText UCDOptionData_Discrete::GetValueText() const
{
	const int32 index = GetIndex();
	return options.IsValidIndex(index) ? options[index] : customText;
}

void UCDOptionData_Discrete::ResetToDefault()
{
	SetIndex(defaultIndex);
}

#undef LOCTEXT_NAMESPACE
