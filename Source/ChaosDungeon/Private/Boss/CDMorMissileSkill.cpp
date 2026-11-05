#include "Boss/CDMorMissileSkill.h"

#include "AIController.h"
#include "AbilitySystem/CDProjectileBase.h"
#include "Character/CDCharacterBase.h"
#include "Data/CDSkillData.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UCDMorMissileSkill::ApplyHit()
{
	ACDCharacterBase* boss = GetCDCharacter();
	UWorld* world = GetWorld();

	if (!IsValid(boss) || !IsValid(world) || !IsValid(skillData))
	{
		return;
	}

	// 데이터 에셋에 BP_MorMissileProjectile 지정
	if (!skillData->projectileData.projectileClass)
	{
		return;
	}

	const AAIController* aiController =
		boss->GetController<AAIController>();

	AActor* target =
		aiController ? aiController->GetFocusActor() : nullptr;

	if (!IsValid(target) ||
		riseSpeed <= 0.f ||
		skillData->projectileData.speed <= 0.f)
	{
		return;
	}

	const int32 count =
		skillData->projectileData.bossData.projectileCount;

	const float riseDuration = FMath::Max(
		skillData->projectileData.bossData.redirectDelay, 0.01f);

	const float interval = FMath::Max(
		skillData->projectileData.bossData.Interval, 0.f);

	const float fireSpeed = skillData->projectileData.speed;
	const FGameplayTag cueTag = launchCueTag;

	const FVector origin =
		boss->GetActorLocation() + FVector::UpVector * spawnHeight;

	const FVector right = boss->GetActorRightVector();
	const TWeakObjectPtr<AActor> weakTarget = target;

	for (int32 index = 0; index < count; ++index)
	{
		const float waitDuration = index * interval;

		FCDProjectileBaseData launchData =
			skillData->projectileData;

		const float sideOffset =
			(index - (count - 1) * 0.5f) * missileSpacing;

		launchData.launchLocation = origin + right * sideOffset;
		launchData.targetPoint =
			launchData.launchLocation + FVector::UpVector;

		launchData.targetActor = target;
		launchData.projectileType = ECDProjectileType::Straight;
		launchData.speed = riseSpeed;

		// 상승·대기 중에는 캐릭터 명중 검사 중지
		launchData.bHitActorsFlight = false;
		launchData.bCausesHitReact = skillData->bCausesHitReact;
		launchData.knockbackStrength = skillData->knockbackStrength;

		// 상승·대기 시간을 수명에 추가
		if (launchData.lifeTime > 0.f)
		{
			launchData.lifeTime += riseDuration + waitDuration;
		}

		// 실제 생성되는 클래스는 데이터에 지정한 BP
		ACDProjectileBase* missile = SpawnProjectile(
			launchData, skillData->damageCoefficient);

		if (!IsValid(missile))
		{
			continue;
		}

		const TWeakObjectPtr<ACDProjectileBase> weakMissile = missile;

		// 모든 탄이 같은 시간 동안 상승
		FTimerDelegate riseDelegate = FTimerDelegate::CreateWeakLambda(
			missile,
			[weakMissile, weakTarget, waitDuration, fireSpeed, cueTag]()
			{
				ACDProjectileBase* currentMissile = weakMissile.Get();

				if (!IsValid(currentMissile))
				{
					return;
				}

				currentMissile->PauseFlight();

				// 자기 차례에 현재 플레이어 좌표 확인
				FTimerDelegate launchDelegate =
					FTimerDelegate::CreateWeakLambda(
						currentMissile,
						[weakMissile, weakTarget, fireSpeed, cueTag]()
						{
							ACDProjectileBase* activeMissile =
								weakMissile.Get();

							AActor* currentTarget = weakTarget.Get();

							if (!IsValid(activeMissile))
							{
								return;
							}

							if (!IsValid(currentTarget))
							{
								activeMissile->Destroy();
								return;
							}

							activeMissile->LaunchAtLocation(
								currentTarget->GetActorLocation(),
								fireSpeed,
								cueTag);
						});

				// 첫 탄은 상승 직후 즉시 발사
				if (waitDuration <= KINDA_SMALL_NUMBER)
				{
					launchDelegate.ExecuteIfBound();
					return;
				}

				FTimerHandle launchTimer;
				currentMissile->GetWorldTimerManager().SetTimer(
					launchTimer,
					launchDelegate,
					waitDuration,
					false);
			});

		FTimerHandle riseTimer;
		world->GetTimerManager().SetTimer(
			riseTimer,
			riseDelegate,
			riseDuration,
			false);
	}
}

void UCDMorMissileSkill::ShowReticle()
{
	// 기본 조준 생략
}