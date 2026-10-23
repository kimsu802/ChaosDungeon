// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "CDAbilityTask_ManageReticle.generated.h"

class ACDReticleActor;
class UCDSkillData;

/** 스킬 조준 중 Reticle을 생성하고 위치와 방향을 갱신하는 Ability Task */
UCLASS()
class CHAOSDUNGEON_API UCDAbilityTask_ManageReticle : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	/** Reticle Task 초기화 */
	UCDAbilityTask_ManageReticle();

	/** 초기 데이터 설정 함수 */
	UFUNCTION()
	static UCDAbilityTask_ManageReticle* ManageReticle(UGameplayAbility* owningAbility, const UCDSkillData* inSkillData, const FVector& targetLocation);

	// UGameplayTask::Activate()
	virtual void Activate() override;

	/** GA Delegate용 Task 종료 함수 */
	void TerminateTask();

protected:
	// UGameplayTask::TickTask()
	virtual void TickTask(float DeltaTime) override;
	// UGameplayTask::OnDestroy()
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	/** 스킬 시전자 저장 - 위치 확인용 */
	UPROPERTY()
	AActor* abilityOwner;

	/** BP Reticle 정보 저장 변수 */
	UPROPERTY()
	TSubclassOf<ACDReticleActor> reticleClass;

	/** 생성한 Reticle 저장 변수 */
	UPROPERTY()
	ACDReticleActor* spawnedReticle;

	/** 스킬 정보 */
	UPROPERTY()
	const UCDSkillData* skillData;

	UPROPERTY()
	FVector targetLocation;
};
