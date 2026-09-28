#include "AbilitySystem/CDGameplayEffects.h"
#include "AbilitySystem/CDDamageExecution.h"
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
