// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonListView.h"
#include "CDOptionListView.generated.h"

class UCDOptionData;

/**
 * 옵션 리스트. 아이템(UCDOptionData) 종류마다 다른 엔트리 위젯을 만든다.
 *   UCDOptionData_Scalar     → WBP_OptionEntry_Scalar   (슬라이더)
 *   UCDOptionData_Discrete   → WBP_OptionEntry_Discrete (< 값 >)
 *   UCDOptionData_KeyBinding → WBP_OptionEntry_Key      (키 입력)
 * 디자이너의 Entry Widget Class 는 아무 엔트리 하나를 넣어 두면 되고, 실제로는 entryClasses 가 우선한다.
 */
UCLASS()
class CHAOSDUNGEON_API UCDOptionListView : public UCommonListView
{
	GENERATED_BODY()

protected:
	// UCommonListView::OnGenerateEntryWidgetInternal()
	virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;
	
protected:
	/** 아이템 클래스 → 엔트리 위젯 (부모 클래스로도 찾음) */
	UPROPERTY(EditAnywhere, Category = "Option")
	TMap<TSubclassOf<class UCDOptionData>, TSubclassOf<class UUserWidget>> entryClasses;

};
