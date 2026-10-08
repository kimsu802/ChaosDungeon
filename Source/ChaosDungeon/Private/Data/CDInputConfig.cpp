#include "Data/CDInputConfig.h"
#include "InputAction.h"
#include "PlayerMappableKeySettings.h"

const UInputAction* UCDInputConfig::FindAbilityAction(FGameplayTag inputTag) const
{
	for (const FCDAbilityInput& abilityInput : abilityInputs)
	{
		if (abilityInput.inputTag == inputTag)
		{
			return abilityInput.action;
		}
	}
	return nullptr;
}

FName UCDInputConfig::FindMappingName(FGameplayTag inputTag) const
{
	const UInputAction* action = FindAbilityAction(inputTag);
	const UPlayerMappableKeySettings* keySettings = action ? action->GetPlayerMappableKeySettings().Get() : nullptr;
	return keySettings ? keySettings->Name : NAME_None;
}
