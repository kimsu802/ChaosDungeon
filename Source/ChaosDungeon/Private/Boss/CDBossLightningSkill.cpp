// Fill out your copyright notice in the Description page of Project Settings.

#include "Boss/CDBossLightningSkill.h"
#include "NavigationSystem.h"
#include <AbilitySystem/CDReticleActor.h>
#include <AbilitySystemComponent.h>
#include <Character/CDBossCharacter.h>
#include <Core/CDGameplayTags.h>
#include <Data/CDBossSkillData.h>
#include <Engine/World.h>


void UCDBossLightningSkill::EndAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo, const FGameplayAbilityActivationInfo activationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(handle, actorInfo))
	{
		return;
	}

	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().ClearTimer(LightningTimers);
	}

	lightningLocations.Reset();
	bLightningScheduled = false;

	// 부모 EndAbility()가 ClearReticle()을 호출한다.
	Super::EndAbility(
		handle,
		actorInfo,
		activationInfo,
		bReplicateEndAbility,
		bWasCancelled);
}


void UCDBossLightningSkill::OnMontageFinished()
{
	if (!IsActive() || bLightningScheduled)
	{
		return;
	}

	UWorld* world = GetWorld();

	if (!world || lightningDelay <= 0.f
		|| lightningLocations.IsEmpty()
		|| spawnedReticle.Num() != lightningLocations.Num())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);

		return;
	}

	bLightningScheduled = true;

	world->GetTimerManager().SetTimer(LightningTimers, this, &ThisClass::LightningTimer, lightningDelay, false);
}

void UCDBossLightningSkill::OnEventReceived(FGameplayEventData payload)
{
	if (!IsActive())
	{
		return;
	}

	const FGameplayTag tag = payload.EventTag;

	if (tag == CDTags::Event_Montage_Hit)
	{
		return;
	}

	if (tag == CDTags::Event_Montage_ReticleOn)
	{
		if (bLightningScheduled || !lightningLocations.IsEmpty())
		{
			return;
		}

		if (!LightningLocation() || !SpawnLightning())
		{
			EndAbility(
				CurrentSpecHandle,
				CurrentActorInfo,
				CurrentActivationInfo,
				true,
				true);
		}
		return;
	}
	Super::OnEventReceived(payload);
}

bool UCDBossLightningSkill::LightningLocation()
{
	lightningLocations.Reset();

	const UCDBossSkillData* data = Cast<UCDBossSkillData>(GetSkillData());

	ACDBossCharacter* boss = Cast<ACDBossCharacter>(GetCDCharacter());

	UWorld* world = GetWorld();

	if (!world || !IsValid(data) || !IsValid(boss)
		|| boss->IsDead())
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

	if (locations.Num() != count)
	{
		return false;
	}

	lightningLocations = locations;

	return true;
}

bool UCDBossLightningSkill::SpawnLightning()
{
	const UCDBossSkillData* data = Cast<UCDBossSkillData>(GetSkillData());

	ACDBossCharacter* boss = Cast<ACDBossCharacter>(GetCDCharacter());

	UWorld* world = GetWorld();

	if (!world || !IsValid(data) || !IsValid(boss) ||
		boss->IsDead() || !reticleClass || lightningLocations.IsEmpty() ||
		data->hitShape.type != ECDHitShapeType::Cone ||
		data->reticleConfig.displayMode == ECDReticleDisplayMode::None)
	{
		return false;
	}

	ClearReticle();

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = boss;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FVector& location : lightningLocations)
	{
		ACDReticleActor* reticle = world->SpawnActor<ACDReticleActor>(reticleClass, location, FRotator::ZeroRotator, spawnParams);

		if (!IsValid(reticle))
		{
			ClearReticle();
			return false;
		}

		// 부모가 제거할 수 있도록 부모 배열에 등록
		spawnedReticle.Add(reticle);

		reticle->SetReticleConfig(*data);
	}

	return true;
}

bool UCDBossLightningSkill::StrikeLightning()
{
	const UCDBossSkillData* data = Cast<UCDBossSkillData>(GetSkillData());

	ACDBossCharacter* boss = Cast<ACDBossCharacter>(GetCDCharacter());

	UWorld* world = GetWorld();

	UAbilitySystemComponent* abilitySystem = GetAbilitySystemComponentFromActorInfo();


	if (!world || !IsValid(data) || !IsValid(boss)
		|| !IsValid(abilitySystem) || boss->IsDead()
		|| lightningLocations.IsEmpty()
		|| data->effectBaseRadius <= 0.f)
	{
		return false;
	}

	FCDHitShape hitShape = data->hitShape;

	const float radius = hitShape.radius;
	const float effectScale = radius / data->effectBaseRadius;

	const FGameplayTag tag = FGameplayTag::RequestGameplayTag(
		FName(TEXT("GameplayCue.Boss.Lightning")));
	
	for (const FVector& location : lightningLocations)
	{
		FGameplayCueParameters params;
		params.Location = location;
		params.RawMagnitude = effectScale;
		params.Instigator = boss;
		params.EffectCauser = boss;

		abilitySystem->ExecuteGameplayCue(tag, params);

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

void UCDBossLightningSkill::LightningTimer()
{
	if (!IsActive() || !bLightningScheduled)
	{
		return;
	}

	// 프레임 지연으로 두 타이머가 같은 프레임에 처리되어도
	// 실제 타격보다 먼저 Reticle이 제거되도록 한다.
	ClearReticle();

	// 낙뢰 실행
	const bool bSucceeded = StrikeLightning();

	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		!bSucceeded);
}
