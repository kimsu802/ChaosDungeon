#include "UI/ListData/CDOptionData_KeyBinding.h"
#include "GameFramework/PlayerController.h"
#include "Settings/CDKeyBindingLibrary.h"

void UCDOptionData_KeyBinding::InitKeyBinding(APlayerController* inPlayerController, FGameplayTag inSlotTag)
{
	playerController = inPlayerController;
	slotTag = inSlotTag;
}

FKey UCDOptionData_KeyBinding::GetKey() const
{
	return UCDKeyBindingLibrary::GetSlotKey(playerController.Get(), slotTag);
}

bool UCDOptionData_KeyBinding::SetKey(const FKey& newKey)
{
	if (IsKeyAllowed(newKey) == false)
	{
		return false;
	}

	if (UCDKeyBindingLibrary::RemapSlotKey(playerController.Get(), slotTag, newKey) == false)
	{
		return false;
	}

	// 맞바꿈으로 다른 슬롯도 바뀌었을 수 있으므로 같은 탭 전체 갱신 (없으면 자기만)
	if (onRemapped)
	{
		onRemapped();
	}
	else
	{
		NotifyValueChanged();
	}
	return true;
}

bool UCDOptionData_KeyBinding::IsKeyAllowed(const FKey& key)
{
	// 메뉴(Esc) · 이동(우클릭) · 클릭(좌클릭) · 조작법(F1) 은 스킬 키로 쓸 수 없음
	static const FKey blockedKeys[] = { EKeys::Escape, EKeys::LeftMouseButton, EKeys::RightMouseButton, EKeys::F1 };
	if (key.IsValid() == false)
	{
		return false;
	}

	for (const FKey& blocked : blockedKeys)
	{
		if (key == blocked)
		{
			return false;
		}
	}
	return true;
}

void UCDOptionData_KeyBinding::ResetToDefault()
{
	const FKey defaultKey = UCDKeyBindingLibrary::GetSlotDefaultKey(playerController.Get(), slotTag);
	if (defaultKey.IsValid() && (defaultKey != GetKey()))
	{
		SetKey(defaultKey);
	}
}
