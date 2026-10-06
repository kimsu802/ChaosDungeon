// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CDBossCharacter.h"

ACDBossCharacter::ACDBossCharacter()
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BossMesh(
		TEXT("/Game/Character/Enemy/ParagonSevarog/Characters/Heroes/Sevarog/Meshes/Sevarog.Sevarog")
	);

	if (BossMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(BossMesh.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> BossAnim(
		TEXT("/Game/Character/Enemy/ParagonSevarog/Characters/Heroes/Sevarog/Sevarog_AnimBlueprint.Sevarog_AnimBlueprint_C")
	);

	if (BossAnim.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(BossAnim.Class);
	}
}

void ACDBossCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACDBossCharacter::BossPattern()
{
}
