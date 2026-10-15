// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CDHitShape.h"
#include "CDReticleActor.generated.h"

UCLASS()
class CHAOSDUNGEON_API ACDReticleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACDReticleActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetReticleConfig(const FCDHitShape& inHitShape);

private:
	UPROPERTY(VisibleAnywhere, Category = "Hit Shape", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDecalComponent> skillReticle;
};
