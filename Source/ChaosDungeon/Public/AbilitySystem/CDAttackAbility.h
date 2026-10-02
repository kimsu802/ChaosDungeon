// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CDGameplayAbility.h"
#include "CDAttackAbility.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDAttackAbility : public UCDGameplayAbility
{
	GENERATED_BODY()
	
public:
	UCDAttackAbility();

	// UGameplayAbility::ActivateAbility()
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData) override;

	// UGameplayAbility::InputPressed()
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	// UCDGameplayAbility::GetCooldownDuration()
	virtual float GetCooldownDuration() const override;

	// UCDGameplayAbility::GetCooldownTag()
	virtual FGameplayTag GetCooldownTag() const override;

private:
	/* 공격 종료 콜백 */
	UFUNCTION()
	void OnCompleteCallback();

	/* 공격 취소 콜백 */
	UFUNCTION()
	void OnInterruptedCallback();

	/* 애님 노티파이 콜백 */
	UFUNCTION()
	void OnAnimNotifyCallback(FGameplayEventData eventData);

private:
	/** 회피 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> montage;

	/* 현재 콤보 카운트 */
	int32 currentCombo = 1;

	/* 콤보 가능 여부 */
	bool bCanCombo = false;
};
