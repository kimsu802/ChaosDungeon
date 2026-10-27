// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Component/CDOptionListView.h"
#include "UI/ListData/CDOptionData.h"

UUserWidget& UCDOptionListView::OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	// 아이템 클래스부터 부모 쪽으로 올라가며 지정된 엔트리를 찾는다 (없으면 디자이너의 Entry Widget Class)
	TSubclassOf<UUserWidget> entryClass = DesiredEntryClass;
	for (UClass* itemClass = Item ? Item->GetClass() : nullptr; itemClass != nullptr; itemClass = itemClass->GetSuperClass())
	{
		if (const TSubclassOf<UUserWidget>* found = entryClasses.Find(itemClass))
		{
			if (*found)
			{
				entryClass = *found;
			}
			break;
		}
	}

	// CommonListView 의 행 생성(버튼 엔트리 지원)은 그대로 사용
	return Super::OnGenerateEntryWidgetInternal(Item, entryClass, OwnerTable);
}
