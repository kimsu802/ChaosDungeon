// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CDMonsterBase.h"
#include <Data/CDBossData.h>
#include "CDBossCharacter.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API ACDBossCharacter : public ACDMonsterBase
{
	GENERATED_BODY()
	
public:
	ACDBossCharacter();

	virtual void BeginPlay() override;

	/** 몬스터 데이터 */
	FORCEINLINE const UCDBossData* GetBossData() const
	{
		return bossData;
	}

	/** 보스의 페이즈 */
	void GetPhaseComponent();

	

protected:
	/** 풀에서 꺼내 데이터/난이도로 초기화 */
	virtual void ActivateFromPool(const UCDMonsterData* data, const FCDDifficultySettings& difficulty, const FVector& location) override;

	/** 사망 연출 후 자동 호출. onReleased 로 스폰 매니저에 반환 */
	virtual void DeactivateToPool() override;

	/** 체력 0 → 사망 처리 + Combat.Death 발행 */
	virtual void HandleDeath() override;

	/** 보스의 패턴을 불러오는 함수 */
	void BossPattern();

	/* 맵에 배치한 보스에게 적용할 데이터 */
	UPROPERTY(EditInstanceOnly, Category = "Boss | Initialization", meta = (DisplayName = "Placed Boss Data"))
	TObjectPtr<UCDMonsterData> placedBossData = nullptr;

protected:
	/**현재 몬스터 데이터 */
	UPROPERTY(Transient)
	TObjectPtr<const UCDBossData> bossData;
};
