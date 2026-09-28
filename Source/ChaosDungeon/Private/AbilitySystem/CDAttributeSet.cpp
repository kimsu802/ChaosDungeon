#include "AbilitySystem/CDAttributeSet.h"
#include "Core/CDGameplayTags.h"
#include "Core/CDTypes.h"
#include "GameplayEffectExtension.h"

void UCDAttributeSet::InitFromStats(const FCDBaseStats& stats, float healthMultiplier, float attackMultiplier)
{
	InitMaxHealth(stats.maxHealth * healthMultiplier);
	InitHealth(GetMaxHealth());
	InitAttackPower(stats.attackPower * attackMultiplier);
	InitDefense(stats.defense);
	InitCritChance(stats.critChance);
	InitCritDamage(stats.critDamage);
}

void UCDAttributeSet::PreAttributeChange(const FGameplayAttribute& attribute, float& newValue)
{
	Super::PreAttributeChange(attribute, newValue);

	if (attribute == GetHealthAttribute())
	{
		newValue = FMath::Clamp(newValue, 0.f, GetMaxHealth());
	}
}

void UCDAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& data)
{
	Super::PostGameplayEffectExecute(data);

	if (data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float damage = GetIncomingDamage();
		SetIncomingDamage(0.f);
		if (damage <= 0.f || GetHealth() <= 0.f)
		{
			return;
		}

		SetHealth(FMath::Clamp(GetHealth() - damage, 0.f, GetMaxHealth()));

		AActor* instigator = data.EffectSpec.GetEffectContext().GetOriginalInstigator();
		const bool bCritical = data.EffectSpec.GetSetByCallerMagnitude(CDTags::SetByCaller_Critical, false, 0.f) > 0.f;
		onDamageTaken.Broadcast(instigator, damage, bCritical);

		if (GetHealth() <= 0.f)
		{
			onOutOfHealth.Broadcast();
		}
	}
	else if (data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// 회복 구슬 등
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}
