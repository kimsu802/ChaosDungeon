// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/CDActivatableScreen.h"
#include "Core/CDTypes.h"
#include "CDConfirmWidget.generated.h"

class UButton;
class UCommonTextBlock;
class UDynamicEntryBox;

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDConfirmWidget : public UCDActivatableScreen
{
	GENERATED_BODY()

public:
	/** Esc/Back 으로 닫히도록 설정 */
	UCDConfirmWidget(const FObjectInitializer& objectInitializer);

	/** 내용, 버튼 구성, 결과 콜백 설정 (활성화 전에 호출) */
	void Setup(ECDConfirmType type, const FText& title, const FText& message, TFunction<void(ECDConfirmResult)> onResult);

protected:
	// UCommonActivatableWidget::NativeGetDesiredFocusTarget()
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	// UCommonActivatableWidget::NativeOnDeactivated()
	virtual void NativeOnDeactivated() override;

private:
	/** 버튼 하나 추가. 누르면 result 로 닫힌다 */
	void AddButton(const FText& label, ECDConfirmResult result);

protected:
	/** 제목 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> titleTextBlock;

	/** 본문 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> messageTextBlock;

	/** 버튼 목록 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> buttonBox;

	/** [확인] 버튼 글자 */
	UPROPERTY(EditDefaultsOnly, Category = "Confirm")
	FText okText = FText::FromString(TEXT("확인"));

	/** [예] 버튼 글자 */
	UPROPERTY(EditDefaultsOnly, Category = "Confirm")
	FText yesText = FText::FromString(TEXT("예"));

	/** [아니오] 버튼 글자 */
	UPROPERTY(EditDefaultsOnly, Category = "Confirm")
	FText noText = FText::FromString(TEXT("아니오"));

	/** [취소] 버튼 글자 */
	UPROPERTY(EditDefaultsOnly, Category = "Confirm")
	FText cancelText = FText::FromString(TEXT("취소"));

private:
	/** 창이 닫힐 때 전달할 결과 (버튼을 누르면 갱신) */
	ECDConfirmResult pendingResult = ECDConfirmResult::Closed;

	/** 결과 콜백 */
	TFunction<void(ECDConfirmResult)> resultCallback;
};
