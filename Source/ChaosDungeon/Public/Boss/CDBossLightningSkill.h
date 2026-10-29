// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CDMonsterSkillAbility.h"
#include "TimerManager.h"
#include "CDBossLightningSkill.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDBossLightningSkill : public UCDMonsterSkillAbility
{
	GENERATED_BODY()

public:
	virtual void EndAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 몽타주 끝나고 나타나도록
	virtual void OnMontageFinished() override;

	virtual void OnEventReceived(FGameplayEventData payload) override;

	virtual void ShowReticle() override {};

private:
	// 라이트닝 위치 저장
	bool LightningLocation();

	// 저장된 위치 Reticle 생성
	bool SpawnLightning();

	// 라이트닝 공격
	bool StrikeLightning();

	// Reticle 제거 후 번개 실행 타이머 함수
	void LightningTimer();

private:
	UPROPERTY(Transient)
	TArray<FVector> lightningLocations;

	FTimerHandle LightningTimers;

	// 종료 처리 중복 방지
	bool bLightningScheduled = false;

	/* 몽타주 끝나고 낙뢰 대기 시간*/
	float lightningDelay = 1.0f;


};
