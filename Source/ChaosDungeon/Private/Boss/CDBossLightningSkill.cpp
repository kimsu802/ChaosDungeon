// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/CDBossLightningSkill.h"
#include <Data/CDBossSkillData.h>
#include <Character/CDBossCharacter.h>
#include <Engine/World.h>
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NavigationSystem.h"


void UCDBossLightningSkill::ApplyHit()
{
}

void UCDBossLightningSkill::OnMontageFinished()
{
	if (!IsActive())
	{
		return;
	}

	if (!StrikeLightning())
	{
		EndAbility(
			CurrentSpecHandle,
			CurrentActorInfo,
			CurrentActivationInfo,
			true,
			true);
		return;
	}

	Super::OnMontageFinished();
}

bool UCDBossLightningSkill::StrikeLightning()
{
	const UCDBossSkillData* data = Cast<UCDBossSkillData>(GetSkillData());

	ACDBossCharacter* boss = Cast<ACDBossCharacter>(GetCDCharacter());

	UWorld* world = GetWorld();

	if (!world || !IsValid(data) || !IsValid(boss)
		|| boss->IsDead()
		|| !IsValid(data->lightningEffect.Get()))
	{
		return false;
	}

	UNavigationSystemV1* navSystem = UNavigationSystemV1::GetCurrent(world);

	if (!navSystem)
	{
		return false;
	}

	const int32 count = data->lightningCount;
	const float radius = data->hitShape.radius;

	// 번개 영역 전체가 생성 범위 안에 들어오도록 제한.
	const float centerRange = data->spawnRadius - radius;

	if (count < 1 || radius <= 0.f || centerRange < 0.f || data->effectBaseRadius <= 0.f)
	{
		return false;
	}

	const FVector center = boss->GetActorLocation();

	const float minDistance = radius * 2.f + FMath::Max(0.f, data->lightningSpacing);

	const float minDistanceSquared = FMath::Square(minDistance);
	const float centerRangeSquared = FMath::Square(centerRange);

	TArray<FVector> locations;

	// 먼저 모든 위치를 확정
	// 무한 반복을 막기 위해 최대 시도 횟수를 제한
	constexpr int32 maxAttempts = 2000;

	for (int32 attempt = 0;
		attempt < maxAttempts && locations.Num() < count;
		++attempt)
	{
		const float angle = FMath::FRandRange(0.f, 2.f * PI);

		// 원 안에서 면적 기준으로 고르게 추출.
		const float distance = FMath::Sqrt(FMath::FRand()) * centerRange;

		const FVector requestedLocation = center + FVector(FMath::Cos(angle), FMath::Sin(angle), 0.f) * distance;

		FNavLocation navLocation;

		if (!navSystem->ProjectPointToNavigation(requestedLocation, navLocation, FVector(75.f, 75.f, 300.f)))
		{
			continue;
		}

		const FVector candidate = navLocation.Location;

		// 내비게이션 보정 후 위치를 기준으로 다시 검사.
		if (FVector::DistSquared2D(center, candidate) > centerRangeSquared)
		{
			continue;
		}

		const bool bOverlaps = locations.ContainsByPredicate([&](const FVector& existing)
			{
				return FVector::DistSquared2D(existing, candidate) < minDistanceSquared;
			});

		if (!bOverlaps)
		{
			locations.Add(candidate);
		}
	}

	// 설정 개수를 확보하지 못하면 이번 낙뢰는 실행하지 않는다.
	if (locations.Num() != count)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Lightning placement failed: %d / %d. "
				"Increase spawn radius or reduce size/count."),
			locations.Num(), count);

		return false;
	}

	FCDHitShape hitShape = data->hitShape;
	hitShape.type = ECDHitShapeType::Circle;

	const float effectScale = radius / data->effectBaseRadius;

	for (const FVector& location : locations)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			world,
			data->lightningEffect.Get(),
			location,
			FRotator::ZeroRotator,
			FVector(effectScale));

		TArray<AActor*> hitActors;

		hitShape.FindActors(
			world,
			location,
			FVector::ForwardVector,
			hitActors);

		for (AActor* actor : hitActors)
		{
			if (IsValid(actor) && boss->IsHostileTo(actor))
			{
				ApplyDamageTo(actor);
			}
		}
	}

	return true;
}
