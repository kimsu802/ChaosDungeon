#include "UI/ListEntry/CDOptionEntryBase.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "UI/ListData/CDOptionData.h"

void UCDOptionEntryBase::NativeOnListItemObjectSet(UObject* listItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(listItemObject);

	UnbindData();

	UCDOptionData* optionData = Cast<UCDOptionData>(listItemObject);
	boundData = optionData;
	if (optionData == nullptr)
	{
		return;
	}

	nameText->SetText(optionData->GetDisplayName());
	valueChangedHandle = optionData->onValueChanged.AddUObject(this, &ThisClass::RefreshValue);

	SetHighlighted(false);
	OnOptionDataSet(optionData);
	RefreshValue();
}

void UCDOptionEntryBase::NativeOnEntryReleased()
{
	UnbindData();
	IUserObjectListEntry::NativeOnEntryReleased();
}

void UCDOptionEntryBase::NativeOnMouseEnter(const FGeometry& inGeometry, const FPointerEvent& inMouseEvent)
{
	Super::NativeOnMouseEnter(inGeometry, inMouseEvent);
	SetHighlighted(true);
}

void UCDOptionEntryBase::NativeOnMouseLeave(const FPointerEvent& inMouseEvent)
{
	Super::NativeOnMouseLeave(inMouseEvent);
	SetHighlighted(false);
}

void UCDOptionEntryBase::OnOptionDataSet(UCDOptionData* optionData)
{
}

void UCDOptionEntryBase::RefreshValue()
{
}

void UCDOptionEntryBase::UnbindData()
{
	if (UCDOptionData* optionData = boundData.Get())
	{
		optionData->onValueChanged.Remove(valueChangedHandle);
	}
	valueChangedHandle.Reset();
	boundData = nullptr;
}

void UCDOptionEntryBase::SetHighlighted(bool bHighlighted)
{
	if (hoverHighlight)
	{
		hoverHighlight->SetRenderOpacity(bHighlighted ? 1.f : 0.f);
	}
}
