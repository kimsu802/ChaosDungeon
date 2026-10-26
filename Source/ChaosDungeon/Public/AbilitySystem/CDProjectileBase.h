// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CDSkillAbility.h"
#include "CDProjectileBase.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class CHAOSDUNGEON_API ACDProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACDProjectileBase();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:
	// 발사체 모양
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TObjectPtr<USkeletalMesh> Mesh;

	// 발사체 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float ProjectileSpeed = 100.f;
};
