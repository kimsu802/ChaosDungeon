// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CDSkillData.h"
#include "CDBossSkillData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class CHAOSDUNGEON_API UCDBossSkillData : public UCDSkillData
{
	GENERATED_BODY()
	
public:
	/* 공격을 사용할 수 있는 최소 거리 */
	UPROPERTY(EditDefaultsOnly, Category = BossAI, meta = (ClampMin = "0"))
	float bossMinRange = 0.0f;

	/* 공격을 사용할 수 있는 최대 거리 */
	UPROPERTY(EditDefaultsOnly, Category = BossAI, meta = (ClampMin = "0"))
	float bossMaxRange = 350.0f;

	/* 공격 우선 순위 (큰 값부터 선택) */
	UPROPERTY(EditDefaultsOnly, Category = BossAI)
	int32 Priority = 0;
};
