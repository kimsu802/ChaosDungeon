#include "AbilitySystem/CDDodgeAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Character/CDCharacterBase.h"
#include "Core/CDGameplayTags.h"

UCDDodgeAbility::UCDDodgeAbility()
{
	SetAssetTags(FGameplayTagContainer(CDTags::Ability_Dodge));
	CancelAbilitiesWithTag.AddTag(CDTags::Ability_Skill);
	ActivationBlockedTags.AddTag(CDTags::State_HitReact);
}

float UCDDodgeAbility::GetCooldownDuration() const
{
	return cooldown;
}

FGameplayTag UCDDodgeAbility::GetCooldownTag() const
{
	return CDTags::Cooldown_Dodge;
}

void UCDDodgeAbility::ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
	const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData)
{
	if (!CommitAbility(handle, actorInfo, activationInfo))
	{
		EndAbility(handle, actorInfo, activationInfo, true, true);
		return;
	}

	ACDCharacterBase* character = GetCDCharacter();
	const FVector aimLocation = character->GetAimLocation();
	BeginCastTowards(aimLocation);
	character->GrantInvincibility(invincibleTime);
	StartDash(aimLocation - character->GetActorLocation(), distance, duration);

	// 몽타주는 연출만, 어빌리티 수명은 이동 시간 기준
	UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, montage, 1.f, NAME_None, false)->ReadyForActivation();

	UAbilityTask_WaitDelay* waitTask = UAbilityTask_WaitDelay::WaitDelay(this, duration);
	waitTask->OnFinish.AddDynamic(this, &ThisClass::OnDodgeFinished);
	waitTask->ReadyForActivation();
}

void UCDDodgeAbility::OnDodgeFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
