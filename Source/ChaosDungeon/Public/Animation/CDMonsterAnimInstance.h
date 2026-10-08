// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CDMonsterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDMonsterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    // UAnimInstance::NativeUpdateAnimation()
    virtual void NativeUpdateAnimation(float deltaSeconds) override;

protected:
    // 몬스터의 수평 이동 속도.
    UPROPERTY(BlueprintReadOnly, Transient, Category = "MonsterMovement")
    float movementSpeed = 0.0f;
};
