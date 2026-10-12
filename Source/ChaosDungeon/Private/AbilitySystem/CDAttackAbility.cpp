// Fill out your copyright notice in the Description page of Project Settings.
#include "AbilitySystem/CDAttackAbility.h"
#include "AbilitySystem/CDGameplayEffects.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Core/CDGameplayTags.h"
#include "Character/CDCharacterBase.h"

UCDAttackAbility::UCDAttackAbility()
{
	SetAssetTags(FGameplayTagContainer(CDTags::Ability_Attack));
	
	// TODO: 공격 취소/ block 조건 추가 
}

void UCDAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actorInfo, const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData * triggerEventData)
{
	if (!CommitAbility(handle, actorInfo, activationInfo))
	{
		UE_LOG(LogTemp, Log, TEXT("fail commit"));
		EndAbility(handle, actorInfo, activationInfo, true, true);
		return;
	}

	bCanCombo = false;
	currentCombo = 1;

	// 몽타주 내 공격 노티파이 수신 기능
	UAbilityTask_WaitGameplayEvent* hitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CDTags::Event_Montage_Hit, nullptr, false);
	hitTask->EventReceived.AddDynamic(this, &ThisClass::OnHitEvent);
	hitTask->ReadyForActivation();


	UAbilityTask_PlayMontageAndWait* playAttackTask =
	UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, montage, 1.0f);
	playAttackTask->OnCompleted.AddDynamic(this, &ThisClass::OnCompleteCallback);
	playAttackTask->OnInterrupted.AddDynamic(this, &ThisClass::OnInterruptedCallback);

	playAttackTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* event = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CDTags::Event_Montage, nullptr, false, false);
	event->EventReceived.AddDynamic(this, &ThisClass::OnAnimNotifyCallback);
	event->ReadyForActivation();
}

void UCDAttackAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	if (bCanCombo)
	{
		bCanCombo = false;

		if (montage->GetNumSections() < currentCombo + 1)
		{
			return;
		}

		currentCombo += 1;
		FName nextSection = FName(*FString::Printf(TEXT("AM_Attack%d"), currentCombo));
		MontageJumpToSection(nextSection);
	}
}

void UCDAttackAbility::ApplyHit()
{
	const ACDCharacterBase* character = GetCDCharacter();
	const FVector origin = character->GetActorLocation();
	const FVector forward = character->GetActorForwardVector();

	TArray<AActor*> hitActors;
	hitShape.FindActors(GetWorld(), origin, forward, hitActors);
	for (AActor* actor : hitActors)
	{
		if (character->IsHostileTo(actor))
		{
			ApplyDamageTo(actor);
		}
	}

	if (CVarDebugHitShape.GetValueOnGameThread())
	{
		hitShape.DrawDebug(GetWorld(), origin, forward, FColor::Red, 1.f);
	}
}

void UCDAttackAbility::ApplyDamageTo(AActor* target) const
{
	UAbilitySystemComponent* targetAbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(target);
	if (!targetAbilitySystem)
	{
		return;
	}

	const FGameplayEffectSpecHandle specHandle = MakeOutgoingGameplayEffectSpec(UCDDamageEffect::StaticClass(), GetAbilityLevel());
	specHandle.Data->SetSetByCallerMagnitude(CDTags::SetByCaller_Coefficient, damageCoefficient);
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*specHandle.Data, targetAbilitySystem);
}

float UCDAttackAbility::GetCooldownDuration() const
{
	return 0.0f;
}

FGameplayTag UCDAttackAbility::GetCooldownTag() const
{
	return FGameplayTag();
}

void UCDAttackAbility::OnHitEvent(FGameplayEventData payload)
{
}

void UCDAttackAbility::OnCompleteCallback()
{
	bCanCombo = false;
	currentCombo = 1;
	
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UCDAttackAbility::OnInterruptedCallback()
{
	UE_LOG(LogTemp, Log, TEXT("OnInterruptedCallback"));
	bCanCombo = false;
	currentCombo = 1;

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

void UCDAttackAbility::OnAnimNotifyCallback(FGameplayEventData eventData)
{
	if (eventData.EventTag == CDTags::Event_Montage_Hit)
	{
		ApplyHit();
	}
	else if (eventData.EventTag == CDTags::Event_Montage_ComboStart)
	{
		bCanCombo = true;
	}
	else if (eventData.EventTag == CDTags::Event_Montage_ComboEnd)
	{
		bCanCombo = false;
	}
}
