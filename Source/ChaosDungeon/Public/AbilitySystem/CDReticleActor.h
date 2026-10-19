// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDReticleActor.generated.h"

class UCDSkillData;

UCLASS()
class CHAOSDUNGEON_API ACDReticleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACDReticleActor();

protected:
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** 장판(Reticle) 설정 함수 */
	void SetReticleConfig(const UCDSkillData* skillData);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Shape")
	TObjectPtr<class UDecalComponent> skillReticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Shape")
	TObjectPtr<class UMaterialInstanceDynamic> reticleMID;
};
