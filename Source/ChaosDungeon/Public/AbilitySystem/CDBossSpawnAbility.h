// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CDMonsterSkillAbility.h"
#include "CDBossSpawnAbility.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDBossSpawnAbility : public UCDMonsterSkillAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ApplyHit() override;

};
