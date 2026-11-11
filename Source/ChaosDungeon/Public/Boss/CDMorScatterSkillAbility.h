// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CDMonsterSkillAbility.h"
#include <AbilitySystem/CDProjectileBase.h>
#include "CDMorScatterSkillAbility.generated.h"

/**
 * Morigesh 흩뿌리기 패턴 어빌리티
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDMorScatterSkillAbility : public UCDMonsterSkillAbility
{
	GENERATED_BODY()
	
public:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* actorInfo, const FGameplayAbilitySpec& spec) override;

	virtual void ApplyHit() override;

protected:
	virtual void FireProjectileWithYaw(float yawOffset) override;

	/** 기본 조준 표시는 생략하고 탄별 착탄 표시를 사용 */
	virtual void ShowReticle() override;

private:
	bool FindRandomPoint(FVector& outLocation);

	/** 지정된 착탄 위치에 표시 생성 */
	ACDReticleActor* SpawnLandingReticle(const FVector& location, ACDProjectileBase* projectile);


private:
	FVector landingLocation = FVector::ZeroVector;

	UPROPERTY()
	FCDProjectileBaseData launchData;

};
