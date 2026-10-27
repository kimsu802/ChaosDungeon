// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ListData/CDOptionData.h"

void UCDOptionData::InitInfo(const FText& inDisplayName, const FText& inDescription)
{
	displayName = inDisplayName;
	description = inDescription;
}

void UCDOptionData::ResetToDefault()
{
}

void UCDOptionData::NotifyValueChanged()
{
	onValueChanged.Broadcast();
}
