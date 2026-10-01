#include "AbilitySystem/CDGameplayAbility.h"
#include "AbilitySystem/CDGameplayEffects.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "AbilitySystemComponent.h"
#include "Character/CDCharacterBase.h"
#include "Core/CDGameplayTags.h"

UCDGameplayAbility::UCDGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	CooldownGameplayEffectClass = UCDCooldownEffect::StaticClass();
	ActivationBlockedTags.AddTag(CDTags::State_Dead);
}

const FGameplayTagContainer* UCDGameplayAbility::GetCooldownTags() const
{
	cooldownTagsCache.Reset();
	const FGameplayTag cooldownTagConst = GetCooldownTag();
	if (cooldownTagConst.IsValid())
	{
		cooldownTagsCache.AddTag(cooldownTagConst);
	}
	return &cooldownTagsCache;
}

void UCDGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
	const FGameplayAbilityActivationInfo activationInfo) const
{
	const float duration = GetCooldownDuration();
	if (duration <= 0.f)
	{
		return;
	}
	const FGameplayEffectSpecHandle specHandle = MakeOutgoingGameplayEffectSpec(handle, actorInfo, activationInfo, CooldownGameplayEffectClass, GetAbilityLevel(handle, actorInfo));
	if (specHandle.IsValid())
	{
		specHandle.Data->SetSetByCallerMagnitude(CDTags::SetByCaller_Cooldown, duration);
		specHandle.Data->DynamicGrantedTags.AddTag(GetCooldownTag());
		ApplyGameplayEffectSpecToOwner(handle, actorInfo, activationInfo, specHandle);
	}
}

float UCDGameplayAbility::GetCooldownDuration() const
{
	return cooldownDuration;
}

FGameplayTag UCDGameplayAbility::GetCooldownTag() const
{
	return cooldownTag;
}

ACDCharacterBase* UCDGameplayAbility::GetCDCharacter() const
{
	return Cast<ACDCharacterBase>(GetAvatarActorFromActorInfo());
}

void UCDGameplayAbility::BeginCastTowards(const FVector& location) const
{
	ACDCharacterBase* character = GetCDCharacter();
	if (AController* controller = character->GetController())
	{
		controller->StopMovement();
	}
	const FVector toTarget = location - character->GetActorLocation();
	if (!toTarget.IsNearlyZero())
	{
		character->SetActorRotation(FRotator(0.f, toTarget.Rotation().Yaw, 0.f));
	}
}

void UCDGameplayAbility::StartDash(const FVector& direction, float distance, float duration)
{
	UAbilityTask_ApplyRootMotionConstantForce* task = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this, NAME_None, direction.GetSafeNormal2D(), distance / duration, duration,
		false, nullptr, ERootMotionFinishVelocityMode::SetVelocity, FVector::ZeroVector, 0.f, false);
	task->ReadyForActivation();
}
