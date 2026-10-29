#include "UI/ViewModel/CDAwakenGaugeVM.h"
#include "AbilitySystem/CDAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Settings/CDKeyBindingLibrary.h"
#include "UserSettings/EnhancedInputUserSettings.h"

void UCDAwakenGaugeVM::BeginDestroy()
{
	Unbind();
	UnbindKey();
	Super::BeginDestroy();
}

void UCDAwakenGaugeVM::BindTo(UAbilitySystemComponent* abilitySystem)
{
	Unbind();
	if (abilitySystem == nullptr)
	{
		return;
	}

	boundAbilitySystem = abilitySystem;
	gaugeHandle = abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetAwakenGaugeAttribute()).AddUObject(this, &ThisClass::HandleGaugeChanged);
	maxGaugeHandle = abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetMaxAwakenGaugeAttribute()).AddUObject(this, &ThisClass::HandleMaxGaugeChanged);

	SetMaxGauge(abilitySystem->GetNumericAttribute(UCDAttributeSet::GetMaxAwakenGaugeAttribute()));
	SetCurrentGauge(abilitySystem->GetNumericAttribute(UCDAttributeSet::GetAwakenGaugeAttribute()));
}

void UCDAwakenGaugeVM::Unbind()
{
	if (UAbilitySystemComponent* abilitySystem = boundAbilitySystem.Get())
	{
		abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetAwakenGaugeAttribute()).Remove(gaugeHandle);
		abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetMaxAwakenGaugeAttribute()).Remove(maxGaugeHandle);
	}
	boundAbilitySystem = nullptr;
	gaugeHandle.Reset();
	maxGaugeHandle.Reset();
}

void UCDAwakenGaugeVM::BindKey(UEnhancedInputUserSettings* userSettings, FName mappingName)
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

void UCDAwakenGaugeVM::UnbindKey()
{
	if (UEnhancedInputUserSettings* userSettings = boundKeySettings.Get())
	{
		userSettings->OnSettingsChanged.RemoveDynamic(this, &ThisClass::HandleKeySettingsChanged);
	}
	boundKeySettings = nullptr;
	keyMappingName = NAME_None;
}

void UCDAwakenGaugeVM::SetCurrentGauge(float value)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(currentGauge, value))
	{
		BroadcastDerived();
	}
}

void UCDAwakenGaugeVM::SetMaxGauge(float value)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(maxGauge, value))
	{
		BroadcastDerived();
	}
}

void UCDAwakenGaugeVM::BroadcastDerived()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetGaugePercent);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsReady);
}

void UCDAwakenGaugeVM::HandleGaugeChanged(const FOnAttributeChangeData& data)
{
	SetCurrentGauge(data.NewValue);
}

void UCDAwakenGaugeVM::HandleMaxGaugeChanged(const FOnAttributeChangeData& data)
{
	SetMaxGauge(data.NewValue);
}

void UCDAwakenGaugeVM::RefreshKeyText()
{
	const FKey key = UCDKeyBindingLibrary::FindMappedKey(boundKeySettings.Get(), keyMappingName);

	// FText 는 == 비교가 없으므로 직접 비교 후 알림
	const FText newText = key.IsValid() ? key.GetDisplayName(false) : FText::GetEmpty();
	if (newText.EqualTo(keyText) == false)
	{
		keyText = newText;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(keyText);
	}
}

void UCDAwakenGaugeVM::HandleKeySettingsChanged(UEnhancedInputUserSettings* settings)
{
	RefreshKeyText();
}
