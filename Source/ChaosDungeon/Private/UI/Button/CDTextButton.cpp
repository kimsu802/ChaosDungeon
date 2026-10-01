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
