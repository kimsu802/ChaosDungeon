// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Component/CDOptionListView.h"

UUserWidget& UCDOptionListView::OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	return Super::OnGenerateEntryWidgetInternal(Item, DesiredEntryClass, OwnerTable);
}
