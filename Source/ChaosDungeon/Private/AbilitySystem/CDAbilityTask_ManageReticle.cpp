// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/CDAbilityTask_ManageReticle.h"
#include "AbilitySystem/CDReticleActor.h"
#include "Data/CDSkillData.h"

UCDAbilityTask_ManageReticle::UCDAbilityTask_ManageReticle()
{
}

UCDAbilityTask_ManageReticle* UCDAbilityTask_ManageReticle::ManageReticle(UGameplayAbility* owningAbility, const UCDSkillData* inSkillData)
{
	UCDAbilityTask_ManageReticle* task = NewAbilityTask<UCDAbilityTask_ManageReticle>(owningAbility);
	task->skillData = inSkillData;

	UE_LOG(LogTemp, Log, TEXT("Manage Reticle"));

	return task;
}

void UCDAbilityTask_ManageReticle::Activate()
{
	Super::Activate();
	UE_LOG(LogTemp, Log, TEXT("Activate AT"));

	UWorld* world = GetWorld();
	if (nullptr == world)
	{
		EndTask();
		return;
	}

	spawnedReticle = world->SpawnActor<ACDReticleActor>(ACDReticleActor::StaticClass(), FTransform::Identity);
	if (IsValid(spawnedReticle))
	{
		UE_LOG(LogTemp, Log, TEXT("spawn Reticle"));
		spawnedReticle->SetReticleConfig(skillData);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to spawn Reticle"));
		EndTask();
	}
}

void UCDAbilityTask_ManageReticle::SkillActivate()
{
	EndTask();
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
