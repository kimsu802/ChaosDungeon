// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/CDAbilityTask_ManageReticle.h"
#include "AbilitySystem/CDReticleActor.h"
#include "Data/CDSkillData.h"
#include "Player/CDPlayerController.h"

UCDAbilityTask_ManageReticle::UCDAbilityTask_ManageReticle()
{
	bTickingTask = true;

	static ConstructorHelpers::FClassFinder<ACDReticleActor> reticleClassRef(TEXT("/Game/Character/Player/Abilities/BP_CDReticleActor.BP_CDReticleActor_C"));
	if (reticleClassRef.Succeeded())
	{
		reticleClass = reticleClassRef.Class;
	}
}

UCDAbilityTask_ManageReticle* UCDAbilityTask_ManageReticle::ManageReticle(UGameplayAbility* owningAbility, const UCDSkillData* inSkillData, const FVector& targetLocation)
{
	UCDAbilityTask_ManageReticle* task = NewAbilityTask<UCDAbilityTask_ManageReticle>(owningAbility);
	task->skillData = inSkillData;
	task->abilityOwner = owningAbility->GetAvatarActorFromActorInfo();
	task->targetLocation = targetLocation;

	return task;
}

void UCDAbilityTask_ManageReticle::Activate()
{
	Super::Activate();

	UWorld* world = GetWorld();
	if (nullptr == world || nullptr == abilityOwner)
	{
		EndTask();
		return;
	}

	spawnedReticle = world->SpawnActor<ACDReticleActor>(reticleClass, abilityOwner->GetActorTransform());
	if (IsValid(spawnedReticle))
	{
		spawnedReticle->SetReticleConfig(*skillData);
	}
	else
	{
		EndTask();
	}
}

void UCDAbilityTask_ManageReticle::TerminateTask()
{
	EndTask();
}

void UCDAbilityTask_ManageReticle::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (nullptr == abilityOwner || nullptr == skillData || nullptr == spawnedReticle)
	{
		return;
	}

	const APawn* caster = Cast<APawn>(abilityOwner);
	ACDPlayerController* pc = nullptr != caster ? Cast<ACDPlayerController>(caster->GetController()) : nullptr;

	if (nullptr == pc)
	{
		switch (skillData->skillMode)
		{
			// [영역 지정]
		case ECDSkillTargetingType::Location:
			spawnedReticle->SetActorLocation(targetLocation);
			break;
		case ECDSkillTargetingType::Direction:
		{
			FHitResult result;
			if (pc->GetHitResultUnderCursor(ECC_Visibility, false, result))
			{
				const FVector start = abilityOwner->GetActorLocation();
				const FVector direction = (result.ImpactPoint - start).GetSafeNormal2D();

				if (skillData->hitShape.type == ECDHitShapeType::Cone)
				{
					spawnedReticle->SetActorLocationAndRotation(start, FRotator(0.0f, direction.Rotation().Yaw, 0.0f));
				}
				else if (skillData->hitShape.type == ECDHitShapeType::Line)
				{
					const FVector center = start + direction * (skillData->hitShape.length * 0.5f);
					spawnedReticle->SetActorLocationAndRotation(center, FRotator(0.0f, direction.Rotation().Yaw, 0.0f));
				}
			}
		}
		break;
		}
	}
	else
	{
		switch (skillData->skillMode)
		{
		case ECDSkillTargetingType::Location:
		{
			// [영역 지정]
			// 마우스 위치를 사거리 안으로 제한
			FHitResult result;
			if (pc->GetHitResultUnderCursor(ECC_Visibility, false, result))
			{
				const FVector mouse = result.ImpactPoint;
				const FVector reticle = abilityOwner->GetActorLocation();
			//
			//	const float distSquare = FVector::DistSquared2D(reticle, mouse);
			//	const float maxSquare = skillData->maxRange * skillData->maxRange;
			//
				FVector destination = mouse;
			//
			//	if (distSquare <= maxSquare)
			//	{
			//		destination = mouse;
			//	}
			//	else
			//	{
			//		FVector direction = (mouse - reticle).GetSafeNormal2D();
			//		destination = reticle + direction * skillData->maxRange;
			//	}
			//	// 높이를 마우스 기준으로 변경
			//	//destination.Z = mouse.Z;
			//
				spawnedReticle->SetActorLocation(destination);
				spawnedReticle->SetValidColor(pc->GetNearestNavLocation(destination));
			}

		}
			break;
		case ECDSkillTargetingType::Direction:
		{
			FHitResult result;
			if (pc->GetHitResultUnderCursor(ECC_Visibility, false, result))
			{
				const FVector start = abilityOwner->GetActorLocation();
				const FVector direction = (result.ImpactPoint - start).GetSafeNormal2D();

				if (skillData->hitShape.type == ECDHitShapeType::Cone)
				{
					spawnedReticle->SetActorLocationAndRotation(start, FRotator(0.0f, direction.Rotation().Yaw, 0.0f));
				}
				else if (skillData->hitShape.type == ECDHitShapeType::Line)
				{
					const FVector center = start + direction * (skillData->hitShape.length * 0.5f);
					spawnedReticle->SetActorLocationAndRotation(center, FRotator(0.0f, direction.Rotation().Yaw, 0.0f));
				}
			}
		}
			break;
		}
	}
}

void UCDAbilityTask_ManageReticle::OnDestroy(bool bInOwnerFinished)
{
	if (IsValid(spawnedReticle))
	{
		spawnedReticle->Destroy();
		spawnedReticle = nullptr;
	}

	Super::OnDestroy(bInOwnerFinished);
}
