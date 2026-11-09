// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CDSkillData.h"
#include "CDReticleActor.generated.h"

class UCDSkillData;

UCLASS()
class CHAOSDUNGEON_API ACDReticleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Reticle 컴포넌트 생성 */
	ACDReticleActor();

	// AActor::Tick()
	virtual void Tick(float DeltaTime) override;

	/** 장판(Reticle) 설정 함수 */
	void SetReticleConfig(const UCDSkillData& skillData);

	/** 장판(Reticle) 크기 설정 함수 */
	void SetReticleSize(const float newHorizontalSize, const float newVerticalSize);

	/** 부채꼴 장판(Reticle) 크기 설정 함수 */
	void SetConeReticleSize(const float radius, const float degree);

	void SetValidColor(bool bIsValid);

	/* 지정된 시간 동안 장판 채우는 함수 - yoonsang */
	void StartFill(float duration);

protected:
	// AActor::BeginPlay()
	virtual void BeginPlay() override;

	/** Decal 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Shape")
	class UDecalComponent* skillReticle;

	/** Decal용 Dynamic Material Instance 저장 변수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Shape")
	class UMaterialInstanceDynamic* reticleMID;

	/** HitShape - MaterialInstance 저장 변수 - editor에서 설정*/
	UPROPERTY(EditDefaultsOnly, Category = "Hit Shape")
	TMap<ECDHitShapeType, class UMaterialInstance*> shapeMaterials;

private:
	/* 장판 경과 시간 - yoonsang */
	float fillElapsedTime = 0.f;

	/* 장판을 채우는 시간 - yoonsang */
	float fillDuration = 1.0f;
};
