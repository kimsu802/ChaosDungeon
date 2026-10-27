// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDSkillAbility.h"
#include "CDProjectileBase.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

/* 투사체 정의가 들어간 클래스*/

UCLASS()
class CHAOSDUNGEON_API ACDProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ACDProjectileBase();




protected:
	/* 발사체 모양*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TObjectPtr<USkeletalMesh> Mesh;

	/* 발사체 속도*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float projectileSpeed = 100.f;

	/* 자동 소멸 시간*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move")
	float lifeTime = 2.f;

private:
	/* 발사 시 피해 GE*/
	//TObjectPtr<FGameplayEffectSpecHandle> damageSpec;

	/* 중복 피해방지 여부*/
	bool bHasImpacted = true;
};
