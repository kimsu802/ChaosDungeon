#include "World/CDSpawnManagerComponent.h"
#include "Character/CDMonsterBase.h"
#include "Data/CDMonsterData.h"
#include "Data/CDStageData.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include <Components/CapsuleComponent.h>
#include "CollisionShape.h"
#include <Engine/World.h>

/* 측정을 위한 헤더 추가*/
#include "ProfilingDebugging/CpuProfilerTrace.h"

void UCDSpawnManagerComponent::Initialize(const TArray<FVector>& inSpawnPoints, const UCDStageData* inStage, const FCDDifficultySettings& inDifficulty)
{
	spawnPoints = inSpawnPoints;
	stage = inStage;
	difficulty = inDifficulty;
	spawnCounts.Reset();
	bBossSpawned = false;
}

void UCDSpawnManagerComponent::StartSpawning()
{
	GetWorld()->GetTimerManager().SetTimer(spawnTimer, this, &ThisClass::SpawnWave, stage->spawnInterval, true, 0.f);
}

void UCDSpawnManagerComponent::StopSpawning()
{
	GetWorld()->GetTimerManager().ClearTimer(spawnTimer);
}

ACDMonsterBase* UCDSpawnManagerComponent::SpawnMonsterAt(const UCDMonsterData* data, const FVector& groundLocation)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CD_SpawnMonsterAt);

	UWorld* world = GetWorld();

	if (!world || !IsValid(stage.Get()) || !IsValid(data) || !data->monsterClass)
	{
		return nullptr;
	}

	const bool bIsBoss = data->grade == ECDMonsterGrade::Boss;
	if (bIsBoss && bBossSpawned)
	{
		return nullptr;
	}

	if (aliveCount >= stage->maxAlive)
	{
		return nullptr;
	}

	const ACDMonsterBase* defaults =
		data->monsterClass->GetDefaultObject<ACDMonsterBase>();

	const UCapsuleComponent* capsule =
		defaults ? defaults->GetCapsuleComponent() : nullptr;

	if (!capsule)
	{
		return nullptr;
	}

	const float radius = capsule->GetScaledCapsuleRadius();
	const float halfHeight = capsule->GetScaledCapsuleHalfHeight();

	const FVector capsuleCenter =
		groundLocation + FVector(0.f, 0.f, halfHeight);

	const FCollisionShape shape = FCollisionShape::MakeCapsule(
		FMath::Max(1.f, radius - 1.f),
		FMath::Max(1.f, halfHeight - 1.f));

	if (world->OverlapBlockingTestByProfile(
		capsuleCenter,
		FQuat::Identity,
		capsule->GetCollisionProfileName(),
		shape))
	{
		return nullptr;
	}

	ACDMonsterBase* monster = AcquireMonster(data);

	if (!IsValid(monster))
	{
		return nullptr;
	}

	bBossSpawned |= bIsBoss;
	// 기존 능력치·스킬·활성화 처리 사용
	monster->ActivateFromPool(data, difficulty, groundLocation);

	++aliveCount;
	++spawnCounts.FindOrAdd(data);

	return monster;
}

void UCDSpawnManagerComponent::SpawnWave()
{
	const int32 waveSize = FMath::CeilToInt(stage->spawnPerWave * difficulty.spawnCountMultiplier);
	const int32 count = FMath::Min(waveSize, stage->maxAlive - aliveCount);

	for (int32 i = 0; i < count; ++i)
	{
		const UCDMonsterData* data = PickMonster();
		FVector location;

		if (!data)
		{
			return;
		}

		if (!FindSpawnLocation(location))
		{
			continue;
		}

		SpawnMonsterAt(data, location);
	}
}

const UCDMonsterData* UCDSpawnManagerComponent::PickMonster() const
{
	// maxCount 에 도달하지 않은 항목 중 가중치 랜덤
	TArray<const FCDSpawnEntry*> candidates;
	float totalWeight = 0.f;
	for (const FCDSpawnEntry& entry : stage->spawns)
	{
		if (!IsValid(entry.monster.Get()) ||
			(bBossSpawned && entry.monster->grade == ECDMonsterGrade::Boss))
		{
			continue;
		}

		const int32* spawned = spawnCounts.Find(entry.monster);
		if (entry.maxCount > 0 && spawned && *spawned >= entry.maxCount)
		{
			continue;
		}
		candidates.Add(&entry);
		totalWeight += entry.weight;
	}

	float roll = FMath::FRand() * totalWeight;
	for (const FCDSpawnEntry* entry : candidates)
	{
		roll -= entry->weight;
		if (roll <= 0.f)
		{
			return entry->monster;
		}
	}
	if (candidates.IsEmpty())
	{
		return nullptr;
	}
	return candidates.Last()->monster;
}

bool UCDSpawnManagerComponent::FindSpawnLocation(FVector& outLocation) const
{
	const APawn* player = UGameplayStatics::GetPlayerPawn(this, 0);
	const UNavigationSystemV1* navSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!player || !navSystem)
	{
		return false;
	}

	TArray<FVector> offScreen;
	TArray<FVector> onScreen;
	for (const FVector& point : spawnPoints)
	{
		const float distance = FVector::Dist2D(point, player->GetActorLocation());
		if (distance < minSpawnDistance || distance > maxSpawnDistance)
		{
			continue;
		}
		FNavLocation navLocation;
		if (!navSystem->GetRandomPointInNavigableRadius(point, spawnScatterRadius, navLocation))
		{
			continue;
		}
		if (IsOnScreen(navLocation.Location))
		{
			onScreen.Add(navLocation.Location);
		}
		else
		{
			offScreen.Add(navLocation.Location);
		}
	}

	const TArray<FVector>& candidates = offScreen.IsEmpty() ? onScreen : offScreen;
	if (candidates.IsEmpty())
	{
		return false;
	}
	outLocation = candidates[FMath::RandRange(0, candidates.Num() - 1)];
	return true;
}

bool UCDSpawnManagerComponent::IsOnScreen(const FVector& location) const
{
	const APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector2D screenPosition;
	if (!playerController || !playerController->ProjectWorldLocationToScreen(location, screenPosition))
	{
		return false;
	}
	int32 width = 0;
	int32 height = 0;
	playerController->GetViewportSize(width, height);
	return screenPosition.X >= 0 && screenPosition.X <= width && screenPosition.Y >= 0 && screenPosition.Y <= height;
}

ACDMonsterBase* UCDSpawnManagerComponent::AcquireMonster(const UCDMonsterData* data)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CD_AcquireMonster);

 	FCDMonsterPool& pool = pools.FindOrAdd(data);
	if (!pool.inactive.IsEmpty())
	{
		return pool.inactive.Pop();
	}

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACDMonsterBase* monster = GetWorld()->SpawnActor<ACDMonsterBase>(data->monsterClass, FTransform::Identity, spawnParams);
	monster->onReleased.AddUObject(this, &ThisClass::ReleaseMonster);
	return monster;
}

void UCDSpawnManagerComponent::ReleaseMonster(ACDMonsterBase* monster)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CD_ReleaseMonster);

	--aliveCount;

	/* 폴링이 없을 시 파괴 테스트 - yoonsang */
	if (!bUseMonsterPooling)
	{
		monster->Destroy();

		return;
	}

	pools.FindOrAdd(monster->GetMonsterData()).inactive.Add(monster);
}
