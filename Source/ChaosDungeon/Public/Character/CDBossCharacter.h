// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CDMonsterBase.h"
#include "CDBossCharacter.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API ACDBossCharacter : public ACDMonsterBase
{
	GENERATED_BODY()
	
public:
	ACDBossCharacter();

protected:
	virtual void BeginPlay() override;

	void BossPattern();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Boss)
	int maxHp = 9999;
};
