// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CDTextButton.generated.h"

/**
 * 
 */
class UCommonTextBlock;

/**
 * 글자 하나를 가진 CommonUI 버튼. 확인창 버튼 등 코드에서 글자를 바꿔야 하는 버튼의 베이스.
 * BP 에 labelText 이름의 CommonTextBlock 을 배치한다. (BindWidget)
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDTextButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	/** 버튼 글자 설정 */
	void SetLabel(const FText& label);

protected:
	/** 버튼 글자 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> labelText;
};
