// Fill out your copyright notice in the Description page of Project Settings.
#include "AbilitySystem/CDAttackAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Core/CDGameplayTags.h"


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

float UCDAttackAbility::GetCooldownDuration() const
{
	return 0.0f;
}

FGameplayTag UCDAttackAbility::GetCooldownTag() const
{
	return FGameplayTag();
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
	if (eventData.EventTag == CDTags::Event_Montage_ComboStart)
	{
		bCanCombo = true;
	}
	else if (eventData.EventTag == CDTags::Event_Montage_ComboEnd)
	{
		bCanCombo = false;
	}
}