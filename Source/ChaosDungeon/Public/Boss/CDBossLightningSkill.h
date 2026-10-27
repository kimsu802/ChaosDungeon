// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CDMonsterSkillAbility.h"
#include "CDBossLightningSkill.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDBossLightningSkill : public UCDMonsterSkillAbility
{
	GENERATED_BODY()
	
	// 데미지 재정의
	virtual void ApplyHit() override;

	// 몽타주 끝나고 나타나도록
	virtual void OnMontageFinished() override;

private:
	bool StrikeLightning();

};
