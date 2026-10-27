// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Component/CDCommonTabListWidgetBase.h"
#include "CommonButtonBase.h"
#include "Components/PanelWidget.h"
#include "UI/Button/CDTextButton.h"

void UCDCommonTabListWidgetBase::RequestRegisterTab(const FName& tabId, const FText& tabDisplayName)
{
	if (GetTabButtonBaseByID(tabId) != nullptr)
	{
		return;
	}

	RegisterTab(tabId, tabButtonClass, nullptr);

	if (UCDTextButton* tabButton = Cast<UCDTextButton>(GetTabButtonBaseByID(tabId)))
	{
		tabButton->SetLabel(tabDisplayName);
	}
}

void UCDCommonTabListWidgetBase::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	Super::HandleTabCreation_Implementation(TabNameID, TabButton);

	if (tabButtonBox && TabButton)
	{
		tabButtonBox->AddChild(TabButton);
	}
}

void UCDCommonTabListWidgetBase::HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	if (TabButton)
	{
		TabButton->RemoveFromParent();
	}

	Super::HandleTabRemoval_Implementation(TabNameID, TabButton);
}
