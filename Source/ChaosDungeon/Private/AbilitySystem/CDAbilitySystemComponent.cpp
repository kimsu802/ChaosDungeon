#include "AbilitySystem/CDAbilitySystemComponent.h"
#include "AbilitySystem/CDSkillAbility.h"
#include "Core/CDGameplayTags.h"
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

FGameplayAbilitySpecHandle UCDAbilitySystemComponent::GrantSkill(UCDSkillData* skill, FGameplayTag skillTag)
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
	return GrantAbility(abilityClass, skillTag, skill);
}

void UCDAbilitySystemComponent::AssignInputTag(FGameplayTag skillTag, FGameplayTag inputTag)
{
	if (FGameplayAbilitySpec* spec = FindSpecByInputTag(skillTag))
	{
		spec->GetDynamicSpecSourceTags().AddTag(inputTag);
		MarkAbilitySpecDirty(*spec);
	}
}

void UCDAbilitySystemComponent::AbilityInputPressed(FGameplayTag inputTag)
{
	// 10.02 Jun6 - 공격/스킬 활성화 여부에 따라 호출함수 분기처리 (const 제거)
	//if (const FGameplayAbilitySpec* spec = FindSpecByInputTag(inputTag))
	if (FGameplayAbilitySpec* spec = FindSpecByInputTag(inputTag))
	{
		if (spec->IsActive())
		{
			AbilitySpecInputPressed(*spec);
		}
		else
		{
			TryActivateAbility(spec->Handle);
		}
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

	if (specA || specB)
	{
		onSkillLoadoutChanged.Broadcast();
	}
}

TMap<FGameplayTag, FGameplayTag> UCDAbilitySystemComponent::GetSkillLoadout() const
{
	const FGameplayTagContainer inputFilter(CDTags::Input_Skill);
	const FGameplayTagContainer skillFilter(CDTags::Ability_Skill);

	TMap<FGameplayTag, FGameplayTag> loadout;
	for (const FGameplayAbilitySpec& spec : ActivatableAbilities.Items)
	{
		// Filter: 부모 태그 기준으로 매칭 (Input.Skill.4 → Input.Skill, Ability.Skill.Strike → Ability.Skill)
		const FGameplayTagContainer inputTags = spec.GetDynamicSpecSourceTags().Filter(inputFilter);
		const FGameplayTagContainer skillTags = spec.GetDynamicSpecSourceTags().Filter(skillFilter);
		if (inputTags.IsEmpty() || skillTags.IsEmpty())
		{
			continue;
		}
		loadout.Add(inputTags.First(), skillTags.First());
	}
	return loadout;
}

void UCDAbilitySystemComponent::ApplySkillLoadout(const TMap<FGameplayTag, FGameplayTag>& loadout)
{
	// 기존 슬롯 태그 제거 (회피/평타 등 Input.Skill 이 아닌 입력 태그는 유지)
	const FGameplayTagContainer inputFilter(CDTags::Input_Skill);
	for (FGameplayAbilitySpec& spec : ActivatableAbilities.Items)
	{
		const FGameplayTagContainer inputTags = spec.GetDynamicSpecSourceTags().Filter(inputFilter);
		if (!inputTags.IsEmpty())
		{
			spec.GetDynamicSpecSourceTags().RemoveTags(inputTags);
			MarkAbilitySpecDirty(spec);
		}
	}

	for (const TPair<FGameplayTag, FGameplayTag>& slot : loadout)
	{
		AssignInputTag(slot.Value, slot.Key);
	}
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
