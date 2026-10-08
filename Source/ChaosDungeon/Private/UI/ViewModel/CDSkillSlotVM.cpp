#include "UI/ViewModel/CDSkillSlotVM.h"
#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "Core/CDGameplayTags.h"
#include "Data/CDSkillData.h"
#include "Settings/CDKeyBindingLibrary.h"
#include "UserSettings/EnhancedInputUserSettings.h"

void UCDSkillSlotVM::BeginDestroy()
{
	Unbind();
	UnbindKey();
	Super::BeginDestroy();
}

void UCDSkillSlotVM::BindTo(UCDAbilitySystemComponent* abilitySystem, FGameplayTag inInputTag)
{
	Unbind();
	boundAbilitySystem = abilitySystem;
	inputTag = inInputTag;
	RefreshIcon();

	if (abilitySystem)
	{
		// 0 = 매 프레임. 0.1초 간격이면 쿨다운 채우기가 계단처럼 끊겨 보인다 (텍스트는 정수가 바뀔 때만 알림)
		tickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::TickCooldown), 0.f);
	}
}

void UCDSkillSlotVM::Unbind()
{
	FTSTicker::GetCoreTicker().RemoveTicker(tickerHandle);
	tickerHandle.Reset();
	boundAbilitySystem = nullptr;
}

void UCDSkillSlotVM::BindKey(UEnhancedInputUserSettings* userSettings, FName mappingName)
{
	UnbindKey();
	boundKeySettings = userSettings;
	keyMappingName = mappingName;

	if (userSettings)
	{
		userSettings->OnSettingsChanged.AddUniqueDynamic(this, &ThisClass::HandleKeySettingsChanged);
	}
	RefreshKeyText();
}

void UCDSkillSlotVM::UnbindKey()
{
	if (UEnhancedInputUserSettings* userSettings = boundKeySettings.Get())
	{
		userSettings->OnSettingsChanged.RemoveDynamic(this, &ThisClass::HandleKeySettingsChanged);
	}
	boundKeySettings = nullptr;
	keyMappingName = NAME_None;
}

void UCDSkillSlotVM::RefreshKeyText()
{
	const FKey key = UCDKeyBindingLibrary::FindMappedKey(boundKeySettings.Get(), keyMappingName);

	// FText 는 == 비교가 없으므로 직접 비교 후 알림
	const FText newText = key.IsValid() ? key.GetDisplayName(false) : FText::GetEmpty();
	if (!newText.EqualTo(keyText))
	{
		keyText = newText;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(keyText);
	}
}

void UCDSkillSlotVM::HandleKeySettingsChanged(UEnhancedInputUserSettings* settings)
{
	RefreshKeyText();
}

void UCDSkillSlotVM::Activate()
{
	if (UCDAbilitySystemComponent* abilitySystem = boundAbilitySystem.Get())
	{
		abilitySystem->AbilityInputPressed(inputTag);
	}
}

void UCDSkillSlotVM::SwapWith(UCDSkillSlotVM* other)
{
	UCDAbilitySystemComponent* abilitySystem = boundAbilitySystem.Get();
	const bool bBothSkillSlots = other && inputTag.MatchesTag(CDTags::Input_Skill) && other->inputTag.MatchesTag(CDTags::Input_Skill);
	if (!abilitySystem || !bBothSkillSlots || other == this)
	{
		return;
	}
	abilitySystem->SwapInputTags(inputTag, other->inputTag);
	RefreshIcon();
	other->RefreshIcon();
}

void UCDSkillSlotVM::RefreshIcon()
{
	const UCDAbilitySystemComponent* abilitySystem = boundAbilitySystem.Get();
	const UCDSkillData* skill = abilitySystem ? abilitySystem->GetSkillByInputTag(inputTag) : nullptr;
	UTexture2D* newIcon = skill ? skill->icon.Get() : nullptr;
	UE_MVVM_SET_PROPERTY_VALUE(icon, newIcon);
}

bool UCDSkillSlotVM::TickCooldown(float deltaTime)
{
	float remaining = 0.f;
	float duration = 0.f;
	const UCDAbilitySystemComponent* abilitySystem = boundAbilitySystem.Get();
	const bool bOnCooldown = abilitySystem
		&& abilitySystem->GetCooldownByInputTag(inputTag, remaining, duration)
		&& remaining > 0.f
		&& duration > 0.f;

	const float newPercent = bOnCooldown ? remaining / duration : 0.f;
	UE_MVVM_SET_PROPERTY_VALUE(cooldownPercent, newPercent);

	// FText 는 == 비교가 없으므로 직접 비교 후 알림
	const FText newText = bOnCooldown ? FText::AsNumber(FMath::CeilToInt(remaining)) : FText::GetEmpty();
	if (!newText.EqualTo(cooldownText))
	{
		cooldownText = newText;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(cooldownText);
	}

	// 계속 틱
	return true;
}
