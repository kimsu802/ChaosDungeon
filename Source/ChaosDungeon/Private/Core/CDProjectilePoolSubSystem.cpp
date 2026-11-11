#include "Core/CDProjectilePoolSubSystem.h"
#include "AbilitySystem/CDProjectileBase.h"
#include "AbilitySystem/CDReticleActor.h"
#include "Data/CDSkillData.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UCDProjectilePoolSubSystem::Prewarm(TSubclassOf<ACDProjectileBase> projectileClass, int32 targetCount,
	TSubclassOf<ACDReticleActor> reticleClass, const UCDSkillData* skill)
{
	if (bShuttingDown || !GetWorld()->IsGameWorld() || !projectileClass || targetCount <= 0)
	{
		return;
	}
	FCDProjectilePoolBucket& pool = pools.FindOrAdd(projectileClass);
	pool.targetCount = FMath::Max(pool.targetCount, targetCount);
	if (reticleClass && IsValid(skill))
	{
		pool.reticleClass = reticleClass;
		pool.skill = skill;
	}
	if (!GetWorld()->GetTimerManager().IsTimerActive(prewarmTimer))
	{
		// 루프 타이머는 느린 프레임에서도 프레임당 한 번만 실행한다.
		FTimerManagerTimerParameters parameters;
		parameters.bLoop = true;
		parameters.bMaxOncePerFrame = true;
		GetWorld()->GetTimerManager().SetTimer(prewarmTimer, this, &ThisClass::TickPrewarm, 0.001f, parameters);
	}
}

bool UCDProjectilePoolSubSystem::IsPooledClass(TSubclassOf<ACDProjectileBase> projectileClass) const
{
	return !bShuttingDown && projectileClass && pools.Contains(projectileClass);
}

ACDProjectileBase* UCDProjectilePoolSubSystem::CreateProjectile(TSubclassOf<ACDProjectileBase> projectileClass)
{
	ACDProjectileBase* projectile = GetWorld()->SpawnActorDeferred<ACDProjectileBase>(
		projectileClass, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!IsValid(projectile))
	{
		return nullptr;
	}
	projectile->InitializePool(this);
	projectile->FinishSpawning(FTransform::Identity);
	if (!IsValid(projectile) || projectile->IsActorBeingDestroyed())
	{
		return nullptr;
	}
	projectile->DeactivateToPool();
	FCDProjectilePoolBucket& pool = pools.FindChecked(projectileClass);
	if (pool.reticleClass && IsValid(pool.skill))
	{
		ACDReticleActor* reticle = GetWorld()->SpawnActor<ACDReticleActor>(pool.reticleClass);
		if (IsValid(reticle))
		{
			reticle->SetActorHiddenInGame(true);
			reticle->SetActorEnableCollision(false);
			reticle->SetReticleConfig(*pool.skill, true);
			projectile->SetLandingReticle(reticle);
		}
	}
	pool.actors.Add(projectile);
	return projectile;
}

ACDProjectileBase* UCDProjectilePoolSubSystem::Acquire(TSubclassOf<ACDProjectileBase> projectileClass)
{
	if (!IsPooledClass(projectileClass) || GetWorld()->bIsTearingDown)
	{
		return nullptr;
	}
	FCDProjectilePoolBucket& pool = pools.FindChecked(projectileClass);
	ACDProjectileBase* projectile = nullptr;
	while (!pool.available.IsEmpty() && !projectile)
	{
		ACDProjectileBase* candidate = pool.available.Pop(EAllowShrinking::No);
		if (IsValid(candidate) && !candidate->IsActorBeingDestroyed()) projectile = candidate;
	}
	if (!projectile) projectile = CreateProjectile(projectileClass);
	if (projectile) projectile->bProjectileActive = true;
	return projectile;
}

void UCDProjectilePoolSubSystem::Release(ACDProjectileBase* projectile)
{
	if (bShuttingDown || !IsValid(projectile) || projectile->IsActorBeingDestroyed()
		|| projectile->projectilePool.Get() != this || !projectile->bProjectileActive)
	{
		return;
	}
	projectile->bProjectileActive = false;
	projectile->DeactivateToPool();
	pools.FindChecked(projectile->GetClass()).available.Add(projectile);
}

void UCDProjectilePoolSubSystem::TickPrewarm()
{
	if (bShuttingDown || GetWorld()->bIsTearingDown) return;
	for (auto& entry : pools)
	{
		FCDProjectilePoolBucket& pool = entry.Value;
		pool.actors.RemoveAll([](const auto& actor) { return !IsValid(actor) || actor->IsActorBeingDestroyed(); });
		if (pool.actors.Num() < pool.targetCount)
		{
			if (ACDProjectileBase* projectile = CreateProjectile(entry.Key))
			{
				pool.available.Add(projectile);
				return;
			}
			pool.targetCount = pool.actors.Num();
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(prewarmTimer);
}

void UCDProjectilePoolSubSystem::Deinitialize()
{
	bShuttingDown = true;
	GetWorld()->GetTimerManager().ClearTimer(prewarmTimer);
	for (auto& entry : pools)
	{
		for (ACDProjectileBase* actor : entry.Value.actors)
		{
			if (IsValid(actor)) actor->Destroy();
		}
	}
	pools.Empty();
	Super::Deinitialize();
}
