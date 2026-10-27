// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/CDTextButton.h"
#include "CommonTextBlock.h"

void UCDTextButton::SetLabel(const FText& label)
{
	if (labelText)
	{
		labelText->SetText(label);
	}
}

void UCDTextButton::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	// 노멀, 호버, 눌림 시 텍스트 스타일이 바뀌며 이를 labelText 블록에 수동적으로 적용해주어야 함.
	if (labelText)
	{
		labelText->SetStyle(GetCurrentTextStyleClass());
	}
}
