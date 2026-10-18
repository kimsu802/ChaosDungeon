#include "AbilitySystem/CDSkillAbility.h"
#include "AbilitySystem/CDGameplayEffects.h"
#include "AbilitySystem/CDAbilityTask_ManageReticle.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/CDCharacterBase.h"
#include "Core/CDGameplayTags.h"
#include "Data/CDSkillData.h"

UCDSkillAbility::UCDSkillAbility()
{
	SetAssetTags(FGameplayTagContainer(CDTags::Ability_Skill));
	ActivationBlockedTags.AddTag(CDTags::State_Casting);
	ActivationBlockedTags.AddTag(CDTags::State_HitReact);
	// 후딜 캔슬 시 이전 스킬 종료
	CancelAbilitiesWithTag.AddTag(CDTags::Ability_Skill);
}

void UCDSkillAbility::OnGiveAbility(const FGameplayAbilityActorInfo* actorInfo, const FGameplayAbilitySpec& spec)
{
	Super::OnGiveAbility(actorInfo, spec);
	skillData = Cast<UCDSkillData>(spec.SourceObject.Get());
}

void UCDSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
	const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData)
{
	if (!skillData)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (skillData->skillMode == ECDSkillCastMode::Instant)
	{
		/* 10.13 Jun6 타겟팅 함수의 타겟 지정완료(2차 입력) 시 기존 Activate 활용 위해 기능 실행부 함수화  */
		ActivateSkillAbility();
	}
	else
	{
		FGameplayAbilitySpec* spec = GetCurrentAbilitySpec();
		if (spec)
		{
			spec->GetDynamicSpecSourceTags().AddTag(CDTags::State_Targeting);
		}

		UCDAbilityTask_ManageReticle* manage = UCDAbilityTask_ManageReticle::ManageReticle(this, skillData);
		onConfirmInput.BindUObject(manage, &UCDAbilityTask_ManageReticle::SkillActivate);
		manage->ReadyForActivation();
	}

}

/** 10.13 Jun6 활성화 상태에서 2차 입력 시 호출되는 함수 - 타겟팅 설정 후 스킬 실행 결정 시 이 함수에서 처리 */
void UCDSkillAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	FGameplayAbilitySpec* spec = GetCurrentAbilitySpec();

	// 즉발 스킬은 추가 입력 상황 자체가 말이 안되는 상황
	if (skillData->skillMode == ECDSkillCastMode::Instant || nullptr == spec)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 조준 상태 종료 - 스킬 발사
	if (spec->GetDynamicSpecSourceTags().HasTag(CDTags::State_Targeting))
	{
		spec->GetDynamicSpecSourceTags().RemoveTag(CDTags::State_Targeting);
		onConfirmInput.ExecuteIfBound();
		ActivateSkillAbility();
		onConfirmInput.Unbind();
	}
}

void UCDSkillAbility::EndAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
	const FGameplayAbilityActivationInfo activationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	SetCasting(false);

	FGameplayAbilitySpec* spec = GetCurrentAbilitySpec();
	if (spec)
	{
		spec->GetDynamicSpecSourceTags().RemoveTag(CDTags::State_Targeting);
	}

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
	const ACDCharacterBase* character = GetCDCharacter();
	const FVector origin = skillData->origin == ECDSkillOrigin::Cursor ? targetLocation : character->GetActorLocation();
	const FVector forward = character->GetActorForwardVector();

	TArray<AActor*> hitActors;
	skillData->hitShape.FindActors(GetWorld(), origin, forward, hitActors);
	for (AActor* actor : hitActors)
	{
		if (character->IsHostileTo(actor))
		{
			ApplyDamageTo(actor);
		}
	}

	if (CVarDebugHitShape.GetValueOnGameThread())
	{
		skillData->hitShape.DrawDebug(GetWorld(), origin, forward, FColor::Red, 1.f);
	}
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
	if (payload.EventTag == CDTags::Event_Montage_Hit)
	{
		ApplyHit();
	}
	else
	{
		ACDCharacterBase* character = GetCDCharacter();

		if (payload.EventTag == CDTags::Event_Montage_Land)
		{
			const float airOffset = character->GetActorLocation().Z - preLocation.Z;
			const float destinationZ = targetLocation.Z + character->GetSimpleCollisionHalfHeight() + airOffset;
			character->SetActorLocation(FVector(targetLocation.X, targetLocation.Y, destinationZ));
		}
	}
}

void UCDSkillAbility::OnRecoveryEvent(FGameplayEventData payload)
{
	SetCasting(false);
}

void UCDSkillAbility::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCDSkillAbility::OnMontageCancelled()
{
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

void UCDSkillAbility::ActivateSkillAbility()
{
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const ACDCharacterBase* character = GetCDCharacter();
	preLocation = character->GetActorLocation();
	const FVector aimLocation = character->GetAimLocation();
	targetLocation = preLocation + (aimLocation - preLocation).GetClampedToMaxSize2D(skillData->maxRange);

	BeginCastTowards(aimLocation);
	SetCasting(true);

	// 스킬 시전 시 이동 거리 계산 및 이동 연출
	if (skillData->dashDistance > 0.f)
	{
		StartDash(character->GetActorForwardVector(), skillData->dashDistance, skillData->dashDuration);
	}

	// 몽타주 내 공격 노티파이 수신 기능
	UAbilityTask_WaitGameplayEvent* hitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CDTags::Event_Montage, nullptr, false, false);
	hitTask->EventReceived.AddDynamic(this, &ThisClass::OnEventReceived);
	hitTask->ReadyForActivation();

	// 취소 가능 시, 취소 노티파이 수신 기능
	if (skillData->bRecoveryCancelable)
	{
		UAbilityTask_WaitGameplayEvent* recoveryTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CDTags::Event_Montage_Recovery);
		recoveryTask->EventReceived.AddDynamic(this, &ThisClass::OnRecoveryEvent);
		recoveryTask->ReadyForActivation();
	}

	// 스킬 몽타주 재생
	UAbilityTask_PlayMontageAndWait* montageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, skillData->montage);
	montageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	montageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
	montageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	montageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	montageTask->ReadyForActivation();
}
