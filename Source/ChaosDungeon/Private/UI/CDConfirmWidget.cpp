// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CDConfirmWidget.h"
#include "UI/Button/CDTextButton.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"

UCDConfirmWidget::UCDConfirmWidget(const FObjectInitializer& objectInitializer)
    :Super(objectInitializer)
{
    inputMode = ECDScreenInputMode::Menu;
    bIsBackHandler = true;
}

void UCDConfirmWidget::Setup(ECDConfirmType type, const FText& title, const FText& message, TFunction<void(ECDConfirmResult)> onResult)
{
    if (titleText)
    {
        titleText->SetText(title);
    }
    if (messageText)
    {
        messageText->SetText(message);
    }
    pendingResult = ECDConfirmResult::Closed;
    resultCallback = MoveTemp(onResult);

    // 스택이 위젯 인스턴스를 재사용하므로, 이전 버튼과 클릭 

}

UWidget* UCDConfirmWidget::NativeGetDesiredFocusTarget() const
{
    // 게임패드/키보드 포커스는 마지막 버튼(아니오/취소)에 둔다. 실수로 확정되지 않도록
    const TArray<UUserWidget*>& buttons = buttonBox->GetAllEntries();
    if (buttons.Num() > 0)
    {
        return buttons.Last();
    }
    return Super::NativeGetDesiredFocusTarget();
}

void UCDConfirmWidget::NativeOnDeactivated()
{	
    // 콜백 안에서 새 확인창을 띄워도(인스턴스 재사용) 안전하도록 먼저 꺼내고 비운다
    TFunction<void(ECDConfirmResult)> callback = MoveTemp(resultCallback);
    resultCallback = nullptr;

    Super::NativeOnDeactivated();

    if (callback)
    {
        callback(pendingResult);
    }
}

void UCDConfirmWidget::AddButton(const FText& label, ECDConfirmResult result)
{
    UCDTextButton* button = buttonBox->CreateEntry<UCDTextButton>();
    if (!button)
    {
        return;
    }

    button->SetLabel(label);
    button->OnClicked().AddWeakLambda(this, [this, result]()
        {
            pendingResult = result;
            DeactivateWidget();
        });
}
