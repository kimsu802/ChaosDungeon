// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CDGameplayAbility.h"
#include "Data/CDHitShape.h"
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
	/** 타격 시점(애님 노티파이)마다 호출. 데이터로 표현 못 하는 특수 스킬은 이 함수만 재정의 */
	virtual void ApplyHit();

	/** 대상에게 데미지 GE 적용 + 피격 이벤트 전송 */
	void ApplyDamageTo(AActor* target) const;

	// UCDGameplayAbility::GetCooldownDuration()
	virtual float GetCooldownDuration() const override;

	// UCDGameplayAbility::GetCooldownTag()
	virtual FGameplayTag GetCooldownTag() const override;

private:
	/** Event.Montage.Hit 수신 */
	UFUNCTION()
	void OnHitEvent(FGameplayEventData payload);

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
	/** 공격 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Attack/Montage")
	TObjectPtr<UAnimMontage> montage;

	/** 몽타주 재생 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Attack/Montage")
	float montagePlayRate = 1.0f;

	/** 일반 공격 범위 */
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FCDHitShape hitShape;;

	/** 일반 공격 계수 */
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float damageCoefficient;

	/* 현재 콤보 카운트 */
	int32 currentCombo = 1;

	/* 콤보 가능 여부 */
	bool bCanCombo = false;
};
