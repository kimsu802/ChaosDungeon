// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CDMonsterData.h"
#include "CDBossData.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDBossData : public UCDMonsterData
{
	GENERATED_BODY()

public:
	/* 페이즈 전환 체력 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Boss)
	float phaseHealthRatio = 0.6f;

	/* 페이즈 전환 애니메이션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Boss)
	TObjectPtr<UAnimMontage> phaseMontage;

	/* 2페이즈에 사용될 Mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Boss)
	TObjectPtr<USkeletalMesh> phaseMesh;

	/* 2페이즈에 사용될 스킬 데이터*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Boss)
	TObjectPtr<const UCDBossData> bossData;


};
