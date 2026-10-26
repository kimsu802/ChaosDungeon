// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CDSkillAbility.h"
#include "CDBossSpawnAbility.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDBossSpawnAbility : public UCDSkillAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ApplyHit() override;

};
