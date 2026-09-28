// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CDCharacterBase.h"

// Sets default values
ACDCharacterBase::ACDCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACDCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACDCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACDCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

