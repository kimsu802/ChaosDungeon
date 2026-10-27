// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "CDCommonTabListWidgetBase.generated.h"

class UCDTextButton;
class UPanelWidget;

/**
 * 글자 탭 리스트 (옵션 화면 상단 "그래픽 / 사운드 / 조작").
 * RequestRegisterTab 으로 탭을 만들면 tabButtonClass 버튼이 tabButtonBox 에 차례로 들어간다.
 * 선택 변경은 부모의 OnTabSelected 로 받는다.
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDCommonTabListWidgetBase : public UCommonTabListWidgetBase
{
	GENERATED_BODY()

public:
	/** 탭 등록 (이미 있으면 무시) + 버튼 글자 설정 */
	void RequestRegisterTab(const FName& tabId, const FText& tabDisplayName);

protected:
	// UCommonTabListWidgetBase::HandleTabCreation_Implementation()
	virtual void HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;

	// UCommonTabListWidgetBase::HandleTabRemoval_Implementation()
	virtual void HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;

protected:
	/** 탭 버튼이 들어갈 패널 (Horizontal Box) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> tabButtonBox;

	/** 탭 버튼 위젯 (WBP_OptionTabButton) */
	UPROPERTY(EditAnywhere, Category = "Tab")
	TSubclassOf<UCDTextButton> tabButtonClass;
	
};
