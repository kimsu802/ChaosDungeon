// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Data/CDHitShape.h"
#include "CDAbilityTask_ManageReticle.generated.h"

class ACDReticleActor;

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDAbilityTask_ManageReticle : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UCDAbilityTask_ManageReticle();

	UFUNCTION()
	static UCDAbilityTask_ManageReticle* ManageReticle(UGameplayAbility* owningAbility, const FCDHitShape& inHitShape);

	virtual void Activate() override;

	void SkillActivate();

protected:
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	UPROPERTY()
	TSubclassOf<ACDReticleActor> reticleClass;

	/** 생성한 Reticle 저장 변수 */
	UPROPERTY()
	TObjectPtr<ACDReticleActor> spawnedReticle;

	FCDHitShape hitShape;

};
