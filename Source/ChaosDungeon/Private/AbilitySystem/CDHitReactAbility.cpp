#include "AbilitySystem/CDHitReactAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystemComponent.h"
#include "Character/CDCharacterBase.h"
#include "Core/CDGameplayTags.h"

UCDHitReactAbility::UCDHitReactAbility()
{
	SetAssetTags(FGameplayTagContainer(CDTags::Ability_HitReact));
	ActivationOwnedTags.AddTag(CDTags::State_HitReact);
	ActivationBlockedTags.AddTag(CDTags::State_Immune_Stagger);
	ActivationBlockedTags.AddTag(CDTags::State_Invincible);
	CancelAbilitiesWithTag.AddTag(CDTags::Ability_Skill);
	bRetriggerInstancedAbility = true;

	FAbilityTriggerData trigger;
	trigger.TriggerTag = CDTags::Event_Hit;
	trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(trigger);
}

void UCDHitReactAbility::ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
	const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData)
{
	ACDCharacterBase* character = GetCDCharacter();

	/* 10.08 - Jun6 디버그용 함수 호출 */
	if (triggerEventData && triggerEventData->Instigator)
	{
		if (const AActor* attacker = triggerEventData->Instigator.Get())
		{
			const FString name = attacker->GetName();
			character->DebugOnHitFunction(attacker);
		}
	}

	const bool bKnockback = triggerEventData && triggerEventData->Instigator && triggerEventData->EventMagnitude > 0.f
		&& !GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(CDTags::State_Immune_Knockback);
	if (bKnockback)
	{
		const FVector away = (character->GetActorLocation() - triggerEventData->Instigator->GetActorLocation()).GetSafeNormal2D();
		character->LaunchCharacter(away * triggerEventData->EventMagnitude, true, false);
	}

	UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, character->GetHitReactMontage())->ReadyForActivation();

	UAbilityTask_WaitDelay* waitTask = UAbilityTask_WaitDelay::WaitDelay(this, staggerDuration);
	waitTask->OnFinish.AddDynamic(this, &ThisClass::OnStaggerFinished);
	waitTask->ReadyForActivation();
}

void UCDHitReactAbility::OnStaggerFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
