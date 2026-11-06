// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/CDMorScatterSkillAbility.h"
#include "AbilitySystem/CDReticleActor.h"
#include <AbilitySystem/CDProjectileBase.h>
#include <Data/CDProjectileData.h>
#include <Data/CDSkillData.h>
#include <Character/CDBossCharacter.h>
#include <Engine/World.h>

void UCDMorScatterSkillAbility::FireProjectileWithYaw(float /*yawOffset*/)
{
	ACDBossCharacter* boss = Cast<ACDBossCharacter>(GetCDCharacter());
	UWorld* world = GetWorld();

	if (!IsValid(boss) || !IsValid(world) || !skillData)
	{
		return;
	}

	if (!FindRandomPoint(landingLocation))
	{
		return;
	}

	launchData = skillData->projectileData;
	launchData.bHitActorsFlight = true;
	launchData.launchLocation = boss->GetActorLocation();
	launchData.targetPoint = landingLocation;

	ACDProjectileBase* projectile = SpawnProjectile(launchData, skillData->damageCoefficient);
	if (IsValid(projectile))
	{
		ACDReticleActor* reticle = SpawnLandingReticle(landingLocation);
		projectile->SetLandingReticle(reticle);
	}
}

void UCDMorScatterSkillAbility::ApplyHit()
{
	Super::ApplyHit();
}

void UCDMorScatterSkillAbility::ShowReticle()
{
	// 착탄 표시는 탄마다 생성한다.
}

ACDReticleActor* UCDMorScatterSkillAbility::SpawnLandingReticle(const FVector& location)
{
	UWorld* world = GetWorld();

	if (!IsValid(world) || !IsValid(skillData) || !reticleClass
		|| skillData->reticleConfig.displayMode == ECDReticleDisplayMode::None)
	{
		return nullptr;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = GetCDCharacter();
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACDReticleActor* reticle = world->SpawnActor<ACDReticleActor>(
		reticleClass, location, FRotator::ZeroRotator, spawnParams);
	if (!IsValid(reticle))
	{
		return nullptr;
	}

	reticle->SetReticleConfig(*skillData);

	const float radius = skillData->projectileData.hitRadius;
	reticle->SetReticleSize(radius, radius);

	return reticle;
}

bool UCDMorScatterSkillAbility::FindRandomPoint(FVector& outLocation)
{
	ACDBossCharacter* boss = Cast<ACDBossCharacter>(GetCDCharacter());
	UWorld* world = GetWorld();

	if (!IsValid(skillData))
	{
		return false;
	}

	const float radius = skillData->projectileData.bossData.spawnRadius;

	if (radius <= 0.f)
	{
		return false;
	}

	// 위치 선택
	const float angle = FMath::FRandRange(0.f, 2.f * PI);
	const float distance = FMath::Sqrt(FMath::FRand()) * radius;

	// 반경
	const FVector offset(FMath::Cos(angle) * distance, FMath::Sin(angle) * distance, 0.f);

	// 시전 위치 기준 랜덤 계산
	outLocation = skillCastLocation + offset;

	FVector traceStart = outLocation + FVector(0, 0, 100);
	FVector traceEnd = outLocation - FVector(0, 0, 1000);

	FHitResult hit;

	// 보스는 판정 제외
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(boss);

	if (!IsValid(world) || !IsValid(boss) ||
		!world->LineTraceSingleByChannel(hit, traceStart, traceEnd, ECC_Visibility, queryParams))
	{
		return false;
	}

	outLocation = hit.ImpactPoint;

	return true;
}
