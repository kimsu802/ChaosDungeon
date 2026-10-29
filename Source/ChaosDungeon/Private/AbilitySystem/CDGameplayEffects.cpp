#include "AbilitySystem/CDGameplayEffects.h"
#include "AbilitySystem/CDDamageExecution.h"
#include "AbilitySystem/CDAttributeSet.h"
#include "Core/CDGameplayTags.h"

UCDDamageEffect::UCDDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition execution;
	execution.CalculationClass = UCDDamageExecution::StaticClass();
	Executions.Add(execution);
}

UCDCooldownEffect::UCDCooldownEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat duration;
	duration.DataTag = CDTags::SetByCaller_Cooldown;
	DurationMagnitude = FGameplayEffectModifierMagnitude(duration);
}

UCDBuffEffect::UCDBuffEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat duration;
	duration.DataTag = CDTags::SetByCaller_BuffDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(duration);

	FSetByCallerFloat buffModifier;
	buffModifier.DataTag = CDTags::SetByCaller_Buff;

	FGameplayModifierInfo modifier;
	modifier.Attribute = UCDAttributeSet::GetAttackPowerAttribute();
	modifier.ModifierOp = EGameplayModOp::MultiplyCompound;
	modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(buffModifier);

	Modifiers.Add(modifier);
}

UCDAwakenEffect::UCDAwakenEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat awaken;
	awaken.DataTag = CDTags::SetByCaller_Awakening;

	FGameplayModifierInfo modifier;
	modifier.Attribute = UCDAttributeSet::GetAwakenGaugeAttribute();
	modifier.ModifierOp = EGameplayModOp::AddBase;
	modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(awaken);

	Modifiers.Add(modifier);
}
