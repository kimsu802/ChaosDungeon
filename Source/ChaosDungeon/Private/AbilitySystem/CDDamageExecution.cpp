#include "AbilitySystem/CDDamageExecution.h"
#include "AbilitySystem/CDAttributeSet.h"
#include "Core/CDGameplayTags.h"

namespace
{
	/** 캡처 정의 모음 (한 번만 생성) */
	struct FCDDamageStatics
	{
		FCDDamageStatics()
			: attackPower(UCDAttributeSet::GetAttackPowerAttribute(), EGameplayEffectAttributeCaptureSource::Source, true)
			, critChance(UCDAttributeSet::GetCritChanceAttribute(), EGameplayEffectAttributeCaptureSource::Source, true)
			, critDamage(UCDAttributeSet::GetCritDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true)
			, defense(UCDAttributeSet::GetDefenseAttribute(), EGameplayEffectAttributeCaptureSource::Target, false)
		{
		}

		/** 가해자 공격력 */
		FGameplayEffectAttributeCaptureDefinition attackPower;

		/** 가해자 치명타 확률 */
		FGameplayEffectAttributeCaptureDefinition critChance;

		/** 가해자 치명타 배율 */
		FGameplayEffectAttributeCaptureDefinition critDamage;

		/** 피해자 방어력 */
		FGameplayEffectAttributeCaptureDefinition defense;
	};

	const FCDDamageStatics& DamageStatics()
	{
		static FCDDamageStatics statics;
		return statics;
	}
}

UCDDamageExecution::UCDDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().attackPower);
	RelevantAttributesToCapture.Add(DamageStatics().critChance);
	RelevantAttributesToCapture.Add(DamageStatics().critDamage);
	RelevantAttributesToCapture.Add(DamageStatics().defense);
}

void UCDDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& executionParams,
	FGameplayEffectCustomExecutionOutput& outExecutionOutput) const
{
	const FGameplayEffectSpec& spec = executionParams.GetOwningSpec();

	FAggregatorEvaluateParameters evaluateParams;
	evaluateParams.SourceTags = spec.CapturedSourceTags.GetAggregatedTags();
	evaluateParams.TargetTags = spec.CapturedTargetTags.GetAggregatedTags();

	// 무적(회피 중, 부활 직후)이면 피해 없음
	if (evaluateParams.TargetTags && evaluateParams.TargetTags->HasTag(CDTags::State_Invincible))
	{
		return;
	}

	auto Capture = [&executionParams, &evaluateParams](const FGameplayEffectAttributeCaptureDefinition& definition)
	{
		float value = 0.f;
		executionParams.AttemptCalculateCapturedAttributeMagnitude(definition, evaluateParams, value);
		return value;
	};

	const float coefficient = spec.GetSetByCallerMagnitude(CDTags::SetByCaller_Coefficient, false, 1.f);
	const float defense = FMath::Clamp(Capture(DamageStatics().defense), 0.f, 0.9f);
	float damage = Capture(DamageStatics().attackPower) * coefficient * (1.f - defense);

	if (FMath::FRand() < Capture(DamageStatics().critChance))
	{
		damage *= Capture(DamageStatics().critDamage);
		// 치명타 여부를 AttributeSet(→ 데미지 숫자 UI)까지 전달
		executionParams.GetOwningSpecForPreExecuteMod()->SetSetByCallerMagnitude(CDTags::SetByCaller_Critical, 1.f);
	}

	if (damage > 0.f)
	{
		outExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UCDAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::AddBase, damage));
	}
}
