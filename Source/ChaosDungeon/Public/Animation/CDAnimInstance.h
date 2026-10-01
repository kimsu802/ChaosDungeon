// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CDAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// UAnimInstance::NativeInitializeAnimation()
	virtual void NativeInitializeAnimation() override;
	// UAnimInstance::NativeUpdateAnimation(float deltaSeconds)
	virtual void NativeUpdateAnimation(float deltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	class ACDPlayerCharacter* player = nullptr;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAttacking = false;

	// 캐릭터 이동 속도 및 가속 여부
	UPROPERTY(BlueprintReadOnly)
	float speed = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	bool bIsAccelerating = false;

	// 캐릭터 회전 값
	UPROPERTY(BlueprintReadOnly)
	float roll = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float pitch = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float yaw = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float yawDelta = 0.0f;

	FRotator prevRotation = FRotator();
};
