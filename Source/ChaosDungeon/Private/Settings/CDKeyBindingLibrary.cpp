#include "Settings/CDKeyBindingLibrary.h"
#include "Data/CDInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Player/CDPlayerController.h"
#include "UserSettings/EnhancedInputUserSettings.h"

FKey UCDKeyBindingLibrary::GetSlotKey(const APlayerController* playerController, FGameplayTag slotTag)
{
	const UEnhancedInputUserSettings* userSettings = GetUserSettings(playerController);
	const UCDInputConfig* inputConfig = GetInputConfig(playerController);
	const FName mappingName = inputConfig ? inputConfig->FindMappingName(slotTag) : NAME_None;
	return FindMappedKey(userSettings, mappingName);
}

bool UCDKeyBindingLibrary::RemapSlotKey(const APlayerController* playerController, FGameplayTag slotTag, FKey newKey)
{
	UEnhancedInputUserSettings* userSettings = GetUserSettings(playerController);
	const UCDInputConfig* inputConfig = GetInputConfig(playerController);
	const FName targetName = inputConfig ? inputConfig->FindMappingName(slotTag) : NAME_None;
	if (!userSettings || targetName.IsNone() || !newKey.IsValid())
	{
		return false;
	}

	const FKey oldKey = FindMappedKey(userSettings, targetName);
	if (oldKey == newKey)
	{
		return true;
	}

	// 같은 키를 쓰던 다른 슬롯이 있으면 그 슬롯에 내 이전 키를 준다 (키 맞바꾸기)
	for (const FCDAbilityInput& abilityInput : inputConfig->abilityInputs)
	{
		const FName otherName = inputConfig->FindMappingName(abilityInput.inputTag);
		if (otherName.IsNone() || otherName == targetName)
		{
			continue;
		}
		if (FindMappedKey(userSettings, otherName) == newKey)
		{
			MapKey(userSettings, otherName, oldKey);
		}
	}

	const bool bMapped = MapKey(userSettings, targetName, newKey);
	ApplyAndSave(userSettings);
	return bMapped;
}

FKey UCDKeyBindingLibrary::FindMappedKey(const UEnhancedInputUserSettings* userSettings, FName mappingName)
{
	if (!userSettings || mappingName.IsNone())
	{
		return EKeys::Invalid;
	}

	// 5.8: 키가 아니라 매핑 정보 포인터를 돌려준다 (사용자가 바꾼 키가 없으면 기본 키)
	const FPlayerKeyMapping* mapping = userSettings->FindCurrentMappingForSlot(mappingName, EPlayerMappableKeySlot::First);
	return mapping ? mapping->GetCurrentKey() : EKeys::Invalid;
}

UEnhancedInputUserSettings* UCDKeyBindingLibrary::GetUserSettings(const APlayerController* playerController)
{
	const ULocalPlayer* localPlayer = playerController ? playerController->GetLocalPlayer() : nullptr;
	const UEnhancedInputLocalPlayerSubsystem* inputSubsystem = localPlayer ? localPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	return inputSubsystem ? inputSubsystem->GetUserSettings() : nullptr;
}

const UCDInputConfig* UCDKeyBindingLibrary::GetInputConfig(const APlayerController* playerController)
{
	const ACDPlayerController* cdController = Cast<ACDPlayerController>(playerController);
	return cdController ? cdController->GetInputConfig() : nullptr;
}

bool UCDKeyBindingLibrary::MapKey(UEnhancedInputUserSettings* userSettings, FName mappingName, const FKey& key)
{
	FMapPlayerKeyArgs args;
	args.MappingName = mappingName;
	args.Slot = EPlayerMappableKeySlot::First;
	args.NewKey = key;

	FGameplayTagContainer failureReason;
	userSettings->MapPlayerKey(args, failureReason);
	return failureReason.IsEmpty();
}

void UCDKeyBindingLibrary::ApplyAndSave(UEnhancedInputUserSettings* userSettings)
{
	userSettings->ApplySettings();
	userSettings->SaveSettings();
}
