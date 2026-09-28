// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CDMonsterBase.h"

// Sets default values
ACDMonsterBase::ACDMonsterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACDMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACDMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACDMonsterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

