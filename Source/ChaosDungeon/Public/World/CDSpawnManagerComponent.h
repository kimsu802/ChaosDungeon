#pragma once

#include "Components/ActorComponent.h"
#include "Core/CDTypes.h"
#include "CDSpawnManagerComponent.generated.h"

class ACDMonsterBase;
class UCDMonsterData;
class UCDStageData;

/** 몬스터 데이터 1종의 비활성 풀 */
USTRUCT()
struct CHAOSDUNGEON_API FCDMonsterPool
{
	GENERATED_BODY()

	/** 재사용 대기 중인 몬스터 */
	UPROPERTY()
	TArray<TObjectPtr<ACDMonsterBase>> inactive;
};

/**
 * 몬스터 공급 담당 (스폰 규칙 + 풀링)
 * - 스폰 포인트 "위치 목록"만 받는다 → 맵이 어떻게 만들어졌는지 모름 (A/B플랜 수정 없이 동작)
 * - 규칙: 플레이어와 min~max 거리, 화면 밖 우선, NavMesh 위, 동시 최대 maxAlive
 */
UCLASS(ClassGroup = (CD))
class CHAOSDUNGEON_API UCDSpawnManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 스테이지 시작 전 설정 */
	void Initialize(const TArray<FVector>& inSpawnPoints, const UCDStageData* inStage, const FCDDifficultySettings& inDifficulty);

	/** 웨이브 스폰 시작 */
	void StartSpawning();

	/** 웨이브 스폰 중지 */
	void StopSpawning();

	/** 살아있는 몬스터 수 */
	FORCEINLINE int32 GetAliveCount() const
	{
		return aliveCount;
	}

private:
	/** 웨이브 1회 스폰 */
	void SpawnWave();

	/** 가중치 랜덤으로 몬스터 선택 (maxCount 초과 항목 제외) */
	const UCDMonsterData* PickMonster() const;

	/** 스폰 규칙에 맞는 위치 찾기 */
	bool FindSpawnLocation(FVector& outLocation) const;

	/** 화면 안인가 */
	bool IsOnScreen(const FVector& location) const;

	/** 풀에서 꺼내거나 새로 생성 */
	ACDMonsterBase* AcquireMonster(const UCDMonsterData* data);

	/** 풀로 반환 (몬스터의 onReleased) */
	void ReleaseMonster(ACDMonsterBase* monster);

protected:
	/** 플레이어와의 최소 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float minSpawnDistance = 800.f;

	/** 플레이어와의 최대 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float maxSpawnDistance = 2000.f;

	/** 같은 포인트에 몰리지 않도록 주변 랜덤 오프셋 */
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float spawnScatterRadius = 200.f;

private:
	/** 스폰 포인트 위치 */
	TArray<FVector> spawnPoints;

	/** 현재 스테이지 */
	UPROPERTY(Transient)
	TObjectPtr<const UCDStageData> stage;

	/** 현재 난이도 */
	FCDDifficultySettings difficulty;

	/** 데이터별 비활성 풀 */
	UPROPERTY(Transient)
	TMap<TObjectPtr<const UCDMonsterData>, FCDMonsterPool> pools;

	/** 데이터별 누적 스폰 수 (maxCount 판정) */
	TMap<TObjectPtr<const UCDMonsterData>, int32> spawnCounts;

	/** 살아있는 몬스터 수 */
	int32 aliveCount = 0;

	/** 웨이브 타이머 */
	FTimerHandle spawnTimer;
};
