// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/CDAbilityTask_ManageReticle.h"
#include "AbilitySystem/CDReticleActor.h"

UCDAbilityTask_ManageReticle::UCDAbilityTask_ManageReticle()
{
	static ConstructorHelpers::FClassFinder<ACDReticleActor> reticleRef(TEXT("/Game/Character/Player/Abilities/BP_CDReticleActor.BP_CDReticleActor_C"));
	if (reticleRef.Succeeded())
	{
		reticleClass = reticleRef.Class;
	}
}

UCDAbilityTask_ManageReticle* UCDAbilityTask_ManageReticle::ManageReticle(UGameplayAbility* owningAbility, const FCDHitShape& inHitShape)
{
	UCDAbilityTask_ManageReticle* task = NewAbilityTask<UCDAbilityTask_ManageReticle>(owningAbility);
	task->hitShape = inHitShape;

	UE_LOG(LogTemp, Log, TEXT("Manage Reticle"));

	return task;
}

void UCDAbilityTask_ManageReticle::Activate()
{
	Super::Activate();
	UE_LOG(LogTemp, Log, TEXT("Activate AT"));

	UWorld* world = GetWorld();
	if (nullptr == world || nullptr == reticleClass)
	{
		EndTask();
		return;
	}

	spawnedReticle = world->SpawnActor<ACDReticleActor>(reticleClass, FTransform::Identity);
	if (IsValid(spawnedReticle))
	{
		UE_LOG(LogTemp, Log, TEXT("spawn Reticle"));
		spawnedReticle->SetReticleConfig(hitShape);
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
