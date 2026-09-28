// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CDCharacterBase.h"
#include "CDMonsterBase.generated.h"

UCLASS()
class CHAOSDUNGEON_API ACDMonsterBase : public ACDCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACDMonsterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
