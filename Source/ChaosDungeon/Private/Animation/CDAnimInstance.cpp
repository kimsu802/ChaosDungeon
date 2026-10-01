// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/CDAnimInstance.h"
#include "Character/CDPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "kismet/kismetMathLibrary.h"

void UCDAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	player = Cast<ACDPlayerCharacter>(TryGetPawnOwner());
}

void UCDAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (nullptr == player)
	{
		return;
	}

	// 속도
	speed = TryGetPawnOwner()->GetVelocity().Length();

	// 회전값
	const FRotator rot = UKismetMathLibrary::NormalizedDeltaRotator(player->GetBaseAimRotation(), player->GetActorRotation());
	roll = rot.Roll;
	pitch = rot.Pitch;
	yaw = rot.Yaw;

	// 좌우 기울어짐 정도
	const float targetYaw = UKismetMathLibrary::NormalizedDeltaRotator(prevRotation, player->GetActorRotation()).Yaw / DeltaSeconds;
	yawDelta = UKismetMathLibrary::FInterpTo(yawDelta, targetYaw, DeltaSeconds, 6.0f);
	prevRotation = player->GetActorRotation();

	// 가속 여부
	bIsAccelerating = player->GetCharacterMovement()->GetCurrentAcceleration().Length() > 0;
}
