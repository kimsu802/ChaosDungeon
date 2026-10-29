// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CDSkillData.h"
#include "CDBossSkillData.generated.h"

class UCDMonsterData;
class UNiagaraSystem;

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

	/* 10.25 추가 (안상범) */
	/* 보스 소환 패턴 */
	/* 소환 몹 데이터 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	TObjectPtr<UCDMonsterData> summonMonsterData = nullptr;

	/** 한 번에 소환할 수 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn",
		meta = (ClampMin = "1"))
	int32 summonCount = 3;

	/** 보스랑 떨어진 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn",
		meta = (ClampMin = "0"))
	float summonForwardDistance = 400.f;

	/** 소환 몬스터 사이의 간격 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn",
		meta = (ClampMin = "1"))
	float summonSpacing = 150.f;

	/* 번개 떨어지는 갯수*/
	UPROPERTY(EditDefaultsOnly, Category = "Lightning", meta = (ClampMin = "1"))
	int32 lightningCount = 10;

	/* 범위 */
	UPROPERTY(EditDefaultsOnly, Category = "Lightning", meta = (ClampMin = "1"))
	float spawnRadius = 1200.f;

	/* 번개간 간격*/
	UPROPERTY(EditDefaultsOnly, Category = "Lightning", meta = (ClampMin = "1"))
	float lightningSpacing = 100.f;

	/* 이펙트 바닥 반지름 */
	UPROPERTY(EditDefaultsOnly, Category = "Lightning", meta = (ClampMin = "1"))
	float effectBaseRadius = 100.f;
};
