#include "AbilitySystem/CDSkillAbility.h"
#include "AbilitySystem/CDGameplayEffects.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Core/CDGameplayTags.h"
#include "Data/CDSkillData.h"

UCDSkillAbility::UCDSkillAbility()
{
	// 스킬 기본 세팅만 두고 필요한 태그는 각 자식 클래스에서 추가
	SetAssetTags(FGameplayTagContainer(CDTags::Ability_Skill));
	ActivationBlockedTags.AddTag(CDTags::State_Casting);

	// 몬스터는 피격 시 공격 block 되지 않는다?
	//ActivationBlockedTags.AddTag(CDTags::State_HitReact);
}

void UCDSkillAbility::OnGiveAbility(const FGameplayAbilityActorInfo* actorInfo, const FGameplayAbilitySpec& spec)
{
	Super::OnGiveAbility(actorInfo, spec);
	skillData = Cast<UCDSkillData>(spec.SourceObject.Get());
}

void UCDSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
	const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData)
{
	// 10.26 Jun6 - 자식 클래스 재정의 필요
	Super::ActivateAbility(handle, actorInfo, activationInfo, triggerEventData);
}

void UCDSkillAbility::EndAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
	const FGameplayAbilityActivationInfo activationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	SetCasting(false);
	Super::EndAbility(handle, actorInfo, activationInfo, bReplicateEndAbility, bWasCancelled);
}

float UCDSkillAbility::GetCooldownDuration() const
{
	return skillData ? skillData->cooldown : 0.f;
}

FGameplayTag UCDSkillAbility::GetCooldownTag() const
{
	return skillData ? skillData->cooldownTag : FGameplayTag();
}

void UCDSkillAbility::ApplyHit()
{
	// 10.26 Jun6 - 자식 클래스 재정의 필요
}

void UCDSkillAbility::ApplyDamageTo(AActor* target) const
{
	UAbilitySystemComponent* targetAbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(target);
	if (!targetAbilitySystem)
	{
		return;
	}

	const FGameplayEffectSpecHandle specHandle = MakeOutgoingGameplayEffectSpec(UCDDamageEffect::StaticClass(), GetAbilityLevel());
	specHandle.Data->SetSetByCallerMagnitude(CDTags::SetByCaller_Coefficient, skillData->damageCoefficient);
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*specHandle.Data, targetAbilitySystem);

	if (skillData->bCausesHitReact)
	{
		// 경직/넉백 적용 여부는 받는 쪽(UCDHitReactAbility)의 면역 태그가 결정
		FGameplayEventData payload;
		payload.Instigator = GetAvatarActorFromActorInfo();
		payload.Target = target;
		payload.EventMagnitude = skillData->knockbackStrength;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(target, CDTags::Event_Hit, payload);
	}
}

void UCDSkillAbility::OnEventReceived(FGameplayEventData payload)
{
	// 10.26 Jun6 - 자식 클래스 재정의 필요
}

void UCDSkillAbility::OnRecoveryEvent(FGameplayEventData payload)
{
	SetCasting(false);
}

void UCDSkillAbility::OnMontageFinished()
{
	if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(CDTags::State_Untargetable))
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(CDTags::State_Untargetable);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCDSkillAbility::OnMontageCancelled()
{
	if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(CDTags::State_Untargetable))
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(CDTags::State_Untargetable);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCDSkillAbility::SetCasting(bool bNewCasting)
{
	if (bIsCasting == bNewCasting)
	{
		return;
	}
	bIsCasting = bNewCasting;

	UAbilitySystemComponent* abilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (bIsCasting)
	{
		abilitySystem->AddLooseGameplayTag(CDTags::State_Casting);
	}
	else
	{
		abilitySystem->RemoveLooseGameplayTag(CDTags::State_Casting);
	}
}