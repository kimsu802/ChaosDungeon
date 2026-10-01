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

	void StartComboTimer();
	void CheckComboInput();

private:
	/** 회피 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TArray<TObjectPtr<UAnimMontage>> montages;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	int32 currentCombo = 0;
};
