#include "World/CDSpawnManagerComponent.h"
#include "Character/CDMonsterBase.h"
#include "Data/CDMonsterData.h"
#include "Data/CDStageData.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

void UCDSpawnManagerComponent::Initialize(const TArray<FVector>& inSpawnPoints, const UCDStageData* inStage, const FCDDifficultySettings& inDifficulty)
{
	spawnPoints = inSpawnPoints;
	stage = inStage;
	difficulty = inDifficulty;
	spawnCounts.Reset();
}

void UCDSpawnManagerComponent::StartSpawning()
{
	GetWorld()->GetTimerManager().SetTimer(spawnTimer, this, &ThisClass::SpawnWave, stage->spawnInterval, true, 0.f);
}

void UCDSpawnManagerComponent::StopSpawning()
{
	GetWorld()->GetTimerManager().ClearTimer(spawnTimer);
}

void UCDSpawnManagerComponent::SpawnWave()
{
	const int32 waveSize = FMath::CeilToInt(stage->spawnPerWave * difficulty.spawnCountMultiplier);
	const int32 count = FMath::Min(waveSize, stage->maxAlive - aliveCount);

	for (int32 i = 0; i < count; ++i)
	{
		const UCDMonsterData* data = PickMonster();
		FVector location;
		if (!data || !FindSpawnLocation(location))
		{
			return;
		}

		ACDMonsterBase* monster = AcquireMonster(data);
		monster->ActivateFromPool(data, difficulty, location);
		++aliveCount;
		++spawnCounts.FindOrAdd(data);
	}
}

const UCDMonsterData* UCDSpawnManagerComponent::PickMonster() const
{
	// maxCount 에 도달하지 않은 항목 중 가중치 랜덤
	TArray<const FCDSpawnEntry*> candidates;
	float totalWeight = 0.f;
	for (const FCDSpawnEntry& entry : stage->spawns)
	{
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
	--aliveCount;
	pools.FindOrAdd(monster->GetMonsterData()).inactive.Add(monster);
}
