#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "AbilitySystem/CDSkillAbility.h"
#include "Data/CDSkillData.h"

FGameplayAbilitySpecHandle UCDAbilitySystemComponent::GrantAbility(TSubclassOf<UGameplayAbility> abilityClass, FGameplayTag inputTag, UObject* sourceObject)
{
	if (!abilityClass)
	{
		return FGameplayAbilitySpecHandle();
	}
	FGameplayAbilitySpec spec(abilityClass, 1, INDEX_NONE, sourceObject);
	if (inputTag.IsValid())
	{
		spec.GetDynamicSpecSourceTags().AddTag(inputTag);
	}
	return GiveAbility(spec);
}

FGameplayAbilitySpecHandle UCDAbilitySystemComponent::GrantSkill(UCDSkillData* skill, FGameplayTag inputTag)
{
	if (!skill)
	{
		return FGameplayAbilitySpecHandle();
	}
	TSubclassOf<UGameplayAbility> abilityClass = UCDSkillAbility::StaticClass();
	if (skill->abilityClass)
	{
		// 특수 스킬
		abilityClass = skill->abilityClass;
	}
	return GrantAbility(abilityClass, inputTag, skill);
}

void UCDAbilitySystemComponent::AbilityInputPressed(FGameplayTag inputTag)
{
	if (const FGameplayAbilitySpec* spec = FindSpecByInputTag(inputTag))
	{
		TryActivateAbility(spec->Handle);
	}
}

bool UCDAbilitySystemComponent::TryActivateSkill(const UCDSkillData* skill)
{
	for (const FGameplayAbilitySpec& spec : ActivatableAbilities.Items)
	{
		if (spec.SourceObject.Get() == skill)
		{
			return TryActivateAbility(spec.Handle);
		}
	}
	return false;
}

void UCDAbilitySystemComponent::SwapInputTags(FGameplayTag slotA, FGameplayTag slotB)
{
	FGameplayAbilitySpec* specA = FindSpecByInputTag(slotA);
	FGameplayAbilitySpec* specB = FindSpecByInputTag(slotB);

	auto Retag = [this](FGameplayAbilitySpec* spec, FGameplayTag from, FGameplayTag to)
	{
		if (spec)
		{
			spec->GetDynamicSpecSourceTags().RemoveTag(from);
			spec->GetDynamicSpecSourceTags().AddTag(to);
			MarkAbilitySpecDirty(*spec);
		}
	};
	Retag(specA, slotA, slotB);
	Retag(specB, slotB, slotA);
}

const UCDSkillData* UCDAbilitySystemComponent::GetSkillByInputTag(FGameplayTag inputTag) const
{
	const FGameplayAbilitySpec* spec = FindSpecByInputTag(inputTag);
	if (!spec)
	{
		return nullptr;
	}
	return Cast<UCDSkillData>(spec->SourceObject.Get());
}

bool UCDAbilitySystemComponent::GetCooldownByInputTag(FGameplayTag inputTag, float& outRemaining, float& outDuration) const
{
	const FGameplayAbilitySpec* spec = FindSpecByInputTag(inputTag);
	const UGameplayAbility* ability = spec ? spec->GetPrimaryInstance() : nullptr;
	if (!ability)
	{
		return false;
	}
	ability->GetCooldownTimeRemainingAndDuration(spec->Handle, AbilityActorInfo.Get(), outRemaining, outDuration);
	return true;
}

const FGameplayAbilitySpec* UCDAbilitySystemComponent::FindSpecByInputTag(FGameplayTag inputTag) const
{
	for (const FGameplayAbilitySpec& spec : ActivatableAbilities.Items)
	{
		if (spec.GetDynamicSpecSourceTags().HasTagExact(inputTag))
		{
			return &spec;
		}
	}
	return nullptr;
}

FGameplayAbilitySpec* UCDAbilitySystemComponent::FindSpecByInputTag(FGameplayTag inputTag)
{
	const ThisClass* constThis = this;
	return const_cast<FGameplayAbilitySpec*>(constThis->FindSpecByInputTag(inputTag));
}
