// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/CDMonsterAnimInstance.h"

#include "Character/CDMonsterBase.h"

void UCDMonsterAnimInstance::NativeUpdateAnimation(float deltaSeconds)
{
    Super::NativeUpdateAnimation(deltaSeconds);

    const ACDMonsterBase* monster =
        Cast<ACDMonsterBase>(TryGetPawnOwner());

    if (!IsValid(monster))
    {
        movementSpeed = 0.0f;
        return;
    }

    movementSpeed = monster->GetVelocity().Size2D();
}