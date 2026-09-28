#include "UI/ViewModel/CDHealthVM.h"
#include "AbilitySystem/CDAttributeSet.h"
#include "AbilitySystemComponent.h"

void UCDHealthVM::BindTo(UAbilitySystemComponent* abilitySystem)
{
	Unbind();
	if (!abilitySystem)
	{
		return;
	}
	boundAbilitySystem = abilitySystem;
	healthHandle = abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	maxHealthHandle = abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::HandleMaxHealthChanged);

	SetMaxHealth(abilitySystem->GetNumericAttribute(UCDAttributeSet::GetMaxHealthAttribute()));
	SetCurrentHealth(abilitySystem->GetNumericAttribute(UCDAttributeSet::GetHealthAttribute()));
}

void UCDHealthVM::Unbind()
{
	if (UAbilitySystemComponent* abilitySystem = boundAbilitySystem.Get())
	{
		abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetHealthAttribute()).Remove(healthHandle);
		abilitySystem->GetGameplayAttributeValueChangeDelegate(UCDAttributeSet::GetMaxHealthAttribute()).Remove(maxHealthHandle);
	}
	boundAbilitySystem = nullptr;
}

void UCDHealthVM::SetCurrentHealth(float value)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(currentHealth, value))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

void UCDHealthVM::SetMaxHealth(float value)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(maxHealth, value))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

void UCDHealthVM::HandleHealthChanged(const FOnAttributeChangeData& data)
{
	SetCurrentHealth(data.NewValue);
}

void UCDHealthVM::HandleMaxHealthChanged(const FOnAttributeChangeData& data)
{
	SetMaxHealth(data.NewValue);
}
