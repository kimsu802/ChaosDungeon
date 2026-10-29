
#include "AbilitySystem/CDPlayerSkillAbility.h"
#include "AbilitySystem/CDAbilityTask_ManageReticle.h"
#include "AbilitySystem/CDGameplayEffects.h"
#include "AbilitySystem/CDAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Character/CDCharacterBase.h"
#include "Core/CDGameplayTags.h"
#include "Data/CDSkillData.h"
#include "Player/CDAwakenDirectorComponent.h"
#include "Player/CDSpeechBubbleComponent.h"


UCDPlayerSkillAbility::UCDPlayerSkillAbility()
{
	ActivationBlockedTags.AddTag(CDTags::State_HitReact);

	// 후딜 캔슬 시 이전 스킬 종료
	CancelAbilitiesWithTag.AddTag(CDTags::Ability_Skill);

	// 10.22 Jun6 - 일반 공격 시 스킬 씹히는 현상 방어용
	BlockAbilitiesWithTag.AddTag(CDTags::Ability_Attack);
}

void UCDPlayerSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo, const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData)
{
	const ACDCharacterBase* character = GetCDCharacter();
	if (!skillData || nullptr == character)
	{
		EndAbility(handle, actorInfo, CurrentActivationInfo, true, true);
		return;
	}

	/* 10.27 Jun6 시전 방식 변수 설정 */
	if (skillData->castPolicy == ECDSkillCastPolicy::Immediate)
	{
		ActivateSkillAbility();
	}
	else
	{
		bool bCanActivate = true;

		if (skillData->skillIdTag == CDTags::Ability_Skill_Awakening)
		{
			UAbilitySystemComponent* abilitySystem = GetAbilitySystemComponentFromActorInfo();
			const float awakenGauge = abilitySystem->GetNumericAttribute(UCDAttributeSet::GetAwakenGaugeAttribute());
			const float maxAwakenGauge = abilitySystem->GetNumericAttribute(UCDAttributeSet::GetMaxAwakenGaugeAttribute());

			bCanActivate = (awakenGauge == maxAwakenGauge);
		}

		if (bCanActivate)
		{
			FGameplayAbilitySpec* spec = GetCurrentAbilitySpec();
			if (spec)
			{
				spec->GetDynamicSpecSourceTags().AddTag(CDTags::State_Targeting);
			}

			UCDAbilityTask_ManageReticle* manage = UCDAbilityTask_ManageReticle::ManageReticle(this, skillData, targetLocation);
			onConfirmInput.BindUObject(manage, &UCDAbilityTask_ManageReticle::TerminateTask);
			manage->ReadyForActivation();
		}
		else
		{
			EndAbility(handle, actorInfo, CurrentActivationInfo, true, true);
			return;
		}
	}
}

void UCDPlayerSkillAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	FGameplayAbilitySpec* spec = GetCurrentAbilitySpec();

	// 즉발 스킬은 추가 입력 상황 자체가 말이 안되는 상황
	if (skillData->castPolicy == ECDSkillCastPolicy::Immediate || nullptr == spec)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 유효한 지점인 곳에 한정하여 스킬 시전
	if (CheckIsValidLocation())
	{
		// 조준 상태 종료 - 스킬 발사
		if (spec->GetDynamicSpecSourceTags().HasTag(CDTags::State_Targeting))
		{
			spec->GetDynamicSpecSourceTags().RemoveTag(CDTags::State_Targeting);
			onConfirmInput.ExecuteIfBound();
			ActivateSkillAbility();
			onConfirmInput.Unbind();
		}
	}
}

void UCDPlayerSkillAbility::EndAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo, const FGameplayAbilityActivationInfo activationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	FGameplayAbilitySpec* spec = GetCurrentAbilitySpec();
	if (spec)
	{
		spec->GetDynamicSpecSourceTags().RemoveTag(CDTags::State_Targeting);
		onConfirmInput.ExecuteIfBound();
		onConfirmInput.Unbind();
	}

	// 10.29 각성기 연출: 취소면 즉시 복귀, 정상 종료면 남은 연출(지연 베기 · 복귀)은 끝까지
	if (UCDAwakenDirectorComponent* director = GetAwakenDirector())
	{
		director->NotifyAbilityEnded(bWasCancelled);
	}

	// 10.29 시전 대사: 스킬이 끝나면(정상 · 취소 모두) 내림
	HideCastLine();

	Super::EndAbility(handle, actorInfo, activationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UCDPlayerSkillAbility::CheckIsValidLocation()
{
	const ACDCharacterBase* character = GetCDCharacter();
	const FVector& location = character->GetAimLocation();
	
	return location != FVector::ZeroVector;
}

void UCDPlayerSkillAbility::ApplyHit()
{
	const ACDCharacterBase* character = GetCDCharacter();
	FVector origin = targetLocation;

	switch (skillData->origin)
	{
	case ECDSkillOrigin::TargetLocation:
		origin = targetLocation;
		break;
	case ECDSkillOrigin::Self:
		origin = character->GetActorLocation();
		break;
	case ECDSkillOrigin::CastStartLocation:
		origin = skillCastLocation;
		break;
	}
	const FVector forward = character->GetActorForwardVector();

	TArray<AActor*> hitActors;
	skillData->hitShape.FindActors(GetWorld(), origin, forward, hitActors);
	for (AActor* actor : hitActors)
	{
		if (character->IsHostileTo(actor))
		{
			ApplyDamageTo(actor);

			GatherAwakeningGauge();

			// 데미지 입힘과 동시에 게임플레이 큐로 히트 이펙트 적용
			FGameplayCueParameters params;
			params.Location = actor->GetActorLocation();
			GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(CDTags::GameplayCue_HitEffect, params);
		}
	}

	if (CVarDebugHitShape.GetValueOnGameThread())
	{
		skillData->hitShape.DrawDebug(GetWorld(), origin, forward, FColor::Red, 1.f);
	}
}

void UCDPlayerSkillAbility::OnEventReceived(FGameplayEventData payload)
{
	const FGameplayTag tag = payload.EventTag;

	if (tag.MatchesTag(CDTags::Event_Montage))
	{
		if (tag == CDTags::Event_Montage_Hit)
		{
			if (skillData->skillType == ECDSkillType::Buff)
			{
				// 추후 skill data에 enum 추가하여 분기 처리 - 임시로 데미지 계수에 buff 비율 적용, buff 지속 시간은 cooltime 보다 4초 적게(임시)
				const FGameplayEffectSpecHandle specHandle = MakeOutgoingGameplayEffectSpec(UCDBuffEffect::StaticClass(), GetAbilityLevel());
				specHandle.Data->SetSetByCallerMagnitude(CDTags::SetByCaller_Buff, skillData->damageCoefficient);
				specHandle.Data->SetSetByCallerMagnitude(CDTags::SetByCaller_BuffDuration, skillData->cooldown - 4.0f);
				GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToSelf(*specHandle.Data);
			}
			else
			{
				// 10.29 각성기 연출: 지연 베기 (피격 이펙트가 Impact 화면 위에 나오도록 ApplyHit 보다 먼저)
				if (UCDAwakenDirectorComponent* director = GetAwakenDirector())
				{
					director->EnterPhase(ECDAwakenPhase::Impact);
				}
				ApplyHit();
			}
		}
		else
		{
			ACDCharacterBase* character = GetCDCharacter();

			if (tag == CDTags::Event_Montage_Targetable)
			{
				UAbilitySystemComponent* abilitySystem = GetAbilitySystemComponentFromActorInfo();
				abilitySystem->RemoveLooseGameplayTag(CDTags::State_Untargetable);
			}
			else if (tag == CDTags::Event_Montage_Untargetable)
			{
				UAbilitySystemComponent* abilitySystem = GetAbilitySystemComponentFromActorInfo();
				abilitySystem->AddLooseGameplayTag(CDTags::State_Untargetable);
			}
			else if (tag == CDTags::Event_Montage_Land)
			{
				const float airOffset = character->GetActorLocation().Z - skillCastLocation.Z;
				const float destinationZ = targetLocation.Z + character->GetSimpleCollisionHalfHeight() + airOffset;
				character->SetActorLocation(FVector(targetLocation.X, targetLocation.Y, destinationZ));
			}
			else if (tag == CDTags::Event_Montage_AwakenFreeze)
			{
				// 10.29 각성기 연출: 도착 후 정적 (전부 멈춤 → 다음 Hit 노티파이가 늦게 온다)
				if (UCDAwakenDirectorComponent* director = GetAwakenDirector())
				{
					director->EnterPhase(ECDAwakenPhase::Freeze);
				}
			}
			else if (tag == CDTags::Event_Montage_ReticleOn)
			{
				FVector origin = FVector::Zero();

				switch (skillData->origin)
				{
				case ECDSkillOrigin::TargetLocation:
					origin = targetLocation;
					break;
				case ECDSkillOrigin::Self:
					origin = GetCDCharacter()->GetActorLocation();
					break;
				case ECDSkillOrigin::CastStartLocation:
					origin = skillCastLocation;
					break;
				}

				UCDAbilityTask_ManageReticle* manage = UCDAbilityTask_ManageReticle::ManageReticle(this, skillData, origin);
				onConfirmInput.BindUObject(manage, &UCDAbilityTask_ManageReticle::TerminateTask);
				manage->ReadyForActivation();
			}
			else if (tag == CDTags::Event_Montage_Move)
			{
				// 10.29 각성기 연출: 돌진 (시간 복귀 · 줌 펀치)
				if (UCDAwakenDirectorComponent* director = GetAwakenDirector())
				{
					director->EnterPhase(ECDAwakenPhase::Release);
				}

				// 스킬 시전 시 이동 거리 계산 및 이동 연출
				if (skillData->dashDistance > 0.f)
				{
					StartDash(character->GetActorForwardVector(), skillData->dashDistance, skillData->dashDuration);
				}
			}
		}
	}
	else
	{
		for (const FCDSkillVFX& vfx : skillData->skillEffects)
		{
			if (tag == vfx.VFXCueTag)
			{
				UAbilitySystemComponent* abilitySystem = GetAbilitySystemComponentFromActorInfo();
				FGameplayCueParameters params;

				FVector origin = FVector::Zero();

				switch (skillData->origin)
				{
				case ECDSkillOrigin::TargetLocation:
					origin = targetLocation;
					break;
				case ECDSkillOrigin::Self:
					origin = GetCDCharacter()->GetActorLocation();
					break;
				case ECDSkillOrigin::CastStartLocation:
					origin = skillCastLocation;
					break;
				}

				params.Location = origin;
				// 10.29 이펙트 방향 = 시전자 정면 (각성기 참격선처럼 돌진 방향을 따라야 하는 이펙트용, GCN 스폰 회전에 사용)
				params.Normal = GetCDCharacter()->GetActorForwardVector();

				if (vfx.effectType == ECDSkillVFXType::Burst)
				{
					abilitySystem->ExecuteGameplayCue(tag, params);
				}
				else
				{
					// Looping - AddGameplayCue / RemoveGameplayCue
					// 몽타주와 별도로 생성이 필요한 경우(설치기 등)에 사용
					// abilitySystem->AddGameplayCue(tag, params);
				}

				break;
			}
		}
	}
}

void UCDPlayerSkillAbility::ActivateSkillAbility()
{	
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (skillData->skillIdTag == CDTags::Ability_Skill_Awakening)
	{
		const FGameplayEffectSpecHandle specHandle = MakeOutgoingGameplayEffectSpec(UCDAwakenEffect::StaticClass(), GetAbilityLevel());
		specHandle.Data->SetSetByCallerMagnitude(CDTags::SetByCaller_Awakening, -100.f);
		GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToSelf(*specHandle.Data);

		// 10.29 각성기 연출: 기 모으기 (세상 느려짐, 플레이어는 정상 속도)
		if (UCDAwakenDirectorComponent* director = GetAwakenDirector())
		{
			director->EnterPhase(ECDAwakenPhase::Charge);
		}
	}

	const ACDCharacterBase* character = GetCDCharacter();
	skillCastLocation = character->GetActorLocation();
	const FVector aimLocation = character->GetAimLocation();
	targetLocation = skillCastLocation + (aimLocation - skillCastLocation).GetClampedToMaxSize2D(skillData->maxRange);

	BeginCastTowards(aimLocation);
	SetCasting(true);

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

	/* 10.20 Jun6 - 스킬 시각 효과 */
	UAbilityTask_WaitGameplayEvent* vfxTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CDTags::GameplayCue_Skill, nullptr, false, false);
	vfxTask->EventReceived.AddDynamic(this, &ThisClass::OnEventReceived);
	vfxTask->ReadyForActivation();

	// 10.29 시전 대사: 몽타주(= 보이스 노티파이)와 같은 순간에 표시
	ShowCastLine();

	// 스킬 몽타주 재생
	UAbilityTask_PlayMontageAndWait* montageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, skillData->montage);
	montageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	montageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
	montageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	montageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	montageTask->ReadyForActivation();
}

UCDAwakenDirectorComponent* UCDPlayerSkillAbility::GetAwakenDirector() const
{
	if ((skillData == nullptr) || (skillData->skillIdTag != CDTags::Ability_Skill_Awakening))
	{
		return nullptr;
	}

	const AActor* avatar = GetAvatarActorFromActorInfo();
	return avatar ? avatar->FindComponentByClass<UCDAwakenDirectorComponent>() : nullptr;
}

void UCDPlayerSkillAbility::ShowCastLine()
{
	if ((skillData == nullptr) || skillData->castLine.IsEmpty())
	{
		return;
	}

	const AActor* avatar = GetAvatarActorFromActorInfo();
	if (UCDSpeechBubbleComponent* speech = avatar ? avatar->FindComponentByClass<UCDSpeechBubbleComponent>() : nullptr)
	{
		castLineId = speech->ShowSpeech(skillData->castLine);
	}
}

void UCDPlayerSkillAbility::HideCastLine()
{
	if (castLineId == INDEX_NONE)
	{
		return;
	}

	const AActor* avatar = GetAvatarActorFromActorInfo();
	if (UCDSpeechBubbleComponent* speech = avatar ? avatar->FindComponentByClass<UCDSpeechBubbleComponent>() : nullptr)
	{
		// 그 사이 다른 대사로 바뀌었으면 컴포넌트가 무시한다
		speech->HideSpeech(castLineId);
	}
	castLineId = INDEX_NONE;
}

void UCDPlayerSkillAbility::GatherAwakeningGauge()
{
	// 각성기가 아니면 각성기 게이지 습득
	if (skillData->skillIdTag != CDTags::Ability_Skill_Awakening)
	{
		// 10.28 Jun6 - 각성기 게이지 관련 코드 추가
		const FGameplayEffectSpecHandle specHandle = MakeOutgoingGameplayEffectSpec(UCDAwakenEffect::StaticClass(), GetAbilityLevel());
		specHandle.Data->SetSetByCallerMagnitude(CDTags::SetByCaller_Awakening, 10.f);
		GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToSelf(*specHandle.Data);
	}
}
