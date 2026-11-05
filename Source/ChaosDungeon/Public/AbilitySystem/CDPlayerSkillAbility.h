// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CDSkillAbility.h"
#include "CDPlayerSkillAbility.generated.h"

DECLARE_DELEGATE(FOnConfirmInput);

class UCDAwakenDirectorComponent;

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDPlayerSkillAbility : public UCDSkillAbility
{
	GENERATED_BODY()
	
public:
	UCDPlayerSkillAbility();

	// UGameplayAbility::ActivateAbility()
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData) override;

	// UGameplayAbility::InputPressed()
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	// UGameplayAbility::EndAbility()
	virtual void EndAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	bool CheckIsValidLocation();

protected:
	virtual void ApplyHit() override;

	virtual void OnEventReceived(FGameplayEventData payload) override;

	/** 기존 Activate 함수 별도 호출용 함수 */
	void ActivateSkillAbility();

	virtual void FireProjectile() override;

private:
	void GatherAwakeningGauge();

	/** 각성기일 때만 시전자의 연출 감독 (아니면 null) */
	UCDAwakenDirectorComponent* GetAwakenDirector() const;

	/** 시전 대사 표시 (skillData->castLine) */
	void ShowCastLine();

	/** 이 스킬이 띄운 시전 대사만 내림 */
	void HideCastLine();

private:
	/** 2차 입력 스킬에 대한 확인 입력 시 Task에 정보 전달용 Delegate 변수 */
	FOnConfirmInput onConfirmInput;

	/** 이 스킬이 띄운 시전 대사 번호 (INDEX_NONE = 없음) */
	int32 castLineId = INDEX_NONE;
};
