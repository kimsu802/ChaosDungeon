#pragma once

#include "Engine/DataAsset.h"
#include "CDStageData.generated.h"

class UCDMonsterData;

/** 스폰 항목 */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDSpawnEntry
{
	GENERATED_BODY()

	/** 스폰할 몬스터 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCDMonsterData> monster;

	/** 가중치 (높을수록 자주) */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
	float weight = 1.f;

	/** 이 스테이지에서 최대 스폰 수. 0 = 무제한 (보스는 1) */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
	int32 maxCount = 0;
};

/**
 * 스테이지 1개 = 에셋 1개. (1단계 일반 / 2단계 일반+정예 / 3단계 보스)
 */
UCLASS(BlueprintType)
class CHAOSDUNGEON_API UCDStageData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 스테이지 레벨 */
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> level;

	/** 스폰 목록 */
	UPROPERTY(EditDefaultsOnly)
	TArray<FCDSpawnEntry> spawns;

	/** 처치 게이지 100% 에 필요한 기여도 합 */
	UPROPERTY(EditDefaultsOnly)
	float targetProgress = 100.f;

	/** 동시 최대 몬스터 수 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	int32 maxAlive = 60;

	/** 웨이브 간격(초) */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float spawnInterval = 1.5f;

	/** 웨이브당 스폰 수 (난이도 배율 적용 전) */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	int32 spawnPerWave = 4;
};
