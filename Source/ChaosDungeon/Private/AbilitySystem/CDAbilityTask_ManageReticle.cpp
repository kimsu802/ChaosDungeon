// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/CDAbilityTask_ManageReticle.h"
#include "AbilitySystem/CDReticleActor.h"
#include "Data/CDSkillData.h"

UCDAbilityTask_ManageReticle::UCDAbilityTask_ManageReticle()
{
	bTickingTask = true;

	static ConstructorHelpers::FClassFinder<ACDReticleActor> reticleClassRef(TEXT("/Game/Character/Player/Abilities/BP_CDReticleActor.BP_CDReticleActor_C"));
	if (reticleClassRef.Succeeded())
	{
		reticleClass = reticleClassRef.Class;
	}
}

UCDAbilityTask_ManageReticle* UCDAbilityTask_ManageReticle::ManageReticle(UGameplayAbility* owningAbility, const UCDSkillData* inSkillData)
{
	UCDAbilityTask_ManageReticle* task = NewAbilityTask<UCDAbilityTask_ManageReticle>(owningAbility);
	task->skillData = inSkillData;
	task->abilityOwner = owningAbility->GetAvatarActorFromActorInfo();

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
		UE_LOG(LogTemp, Log, TEXT("spawn Reticle"));
		spawnedReticle->SetReticleConfig(*skillData);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to spawn Reticle"));
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

	APlayerController* pc = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());

	if (nullptr == pc||nullptr == abilityOwner || nullptr == skillData || nullptr == spawnedReticle)
	{
		return;
	}

	// Todo: 여기서 플레이어 컨트롤러 인 지 검사 후 monster 면 실행 -joonsik님 의견
	//APlayerController* pc = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	//if (nullptr == pc)
	//{
	//	플레이어 로직을 넣고 아니라면 몬스터 로직을 넣어
	//}

	switch (skillData->skillMode)
	{
	case ECDSkillMode::AreaSection:
	{
		// [영역 지정]
		// 마우스 위치를 사거리 안으로 제한
		FHitResult result;
		if (pc->GetHitResultUnderCursor(ECC_Visibility, false, result))
		{
			const FVector mouse = result.ImpactPoint;
			const FVector reticle = abilityOwner->GetActorLocation();

			const float distSquare = FVector::DistSquared2D(reticle, mouse);
			const float maxSquare = skillData->maxRange * skillData->maxRange;

			FVector destination = FVector::Zero();

			if (distSquare <= maxSquare)
			{
				destination = mouse;
			}
			else
			{
				FVector direction = (mouse - reticle).GetSafeNormal2D();
				destination = reticle + direction * skillData->maxRange;
			}

			// 높이를 마우스 기준으로 변경
			destination.Z = mouse.Z;
			spawnedReticle->SetActorLocation(destination);
		}
	}
		break;
	case ECDSkillMode::DirectionSelection:
	{
		FHitResult result;
		if (pc->GetHitResultUnderCursor(ECC_Visibility, false, result))
		{
			//const bool bIsSelf = skillData->origin == ECDSkillOrigin::Self;
			//
			//const FVector mouse = result.ImpactPoint;
			//const FVector start = bIsSelf ? abilityOwner->GetActorLocation() : mouse;
			//
			//const FVector direction = (mouse - start).GetSafeNormal2D();
			//const float distance = bIsSelf ? skillData->hitShape.radius : FMath::Clamp(FVector::Dist2D(start, mouse), 0, skillData->maxRange);
			//const FVector end = start + direction * distance;
			//const FVector center = (start + end) * 0.5f;
			//
			//skillData->hitShape.DrawDebug(GetWorld(), start, abilityOwner->GetActorForwardVector(), FColor::Red, 1.f);
			//
			//
			//spawnedReticle->SetActorLocationAndRotation(center, FRotator(0.0f, direction.Rotation().Yaw, 0.0f));
			//spawnedReticle->SetReticleSize(skillData->hitShape.width, distance);

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

void UCDAbilityTask_ManageReticle::OnDestroy(bool bInOwnerFinished)
{
	if (IsValid(spawnedReticle))
	{
		spawnedReticle->Destroy();
		spawnedReticle = nullptr;
	}

	Super::OnDestroy(bInOwnerFinished);
}
