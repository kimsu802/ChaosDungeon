// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/CDProjectileBase.h"

// Sets default values
ACDProjectileBase::ACDProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}


// Called every frame
void ACDProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

