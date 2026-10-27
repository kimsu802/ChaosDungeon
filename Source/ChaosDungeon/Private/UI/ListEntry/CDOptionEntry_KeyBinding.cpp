#include "UI/ListEntry/CDOptionEntry_KeyBinding.h"
#include "CommonTextBlock.h"
#include "Components/InputKeySelector.h"
#include "UI/ListData/CDOptionData_KeyBinding.h"

void UCDOptionEntry_KeyBinding::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 스킬 키는 단일 키만 (Shift+Q 같은 조합 · 패드 버튼 제외), Esc 는 입력 취소
	keySelector->SetAllowModifierKeys(false);
	keySelector->SetAllowGamepadKeys(false);
	keySelector->SetEscapeKeys({ EKeys::Escape });
	keySelector->OnKeySelected.AddDynamic(this, &ThisClass::HandleKeySelected);
	keySelector->OnIsSelectingKeyChanged.AddDynamic(this, &ThisClass::HandleSelectingChanged);

	if (selectingHintText)
	{
		selectingHintText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCDOptionEntry_KeyBinding::RefreshValue()
{
	if (const UCDOptionData_KeyBinding* keyData = GetKeyData())
	{
		keySelector->SetSelectedKey(FInputChord(keyData->GetKey()));
	}
}

void UCDOptionEntry_KeyBinding::HandleKeySelected(FInputChord selectedKey)
{
	UCDOptionData_KeyBinding* keyData = GetKeyData();
	if (keyData == nullptr)
	{
		return;
	}

	// 막힌 키 · 실패 → 원래 키로 표시 복구 (성공하면 데이터가 알림 → RefreshValue)
	if (keyData->SetKey(selectedKey.Key) == false)
	{
		RefreshValue();
	}
}

void UCDOptionEntry_KeyBinding::HandleSelectingChanged()
{
	if (selectingHintText)
	{
		selectingHintText->SetVisibility(keySelector->GetIsSelectingKey() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

UCDOptionData_KeyBinding* UCDOptionEntry_KeyBinding::GetKeyData() const
{
	return Cast<UCDOptionData_KeyBinding>(GetOptionData());
}
