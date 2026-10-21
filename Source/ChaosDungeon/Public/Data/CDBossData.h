// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CDMonsterData.h"
#include "CDBossData.generated.h"

/**
 * 보스 전용 데이터. 일반 몬스터 데이터에 보스 HP 바 표시 정보를 더한다.
 * 보스 등장 시 UCDStageVM 이 읽어 보스 HP 바(이름 · 줄 수)에 반영한다.
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

	/** 보스 HP 바에 표시할 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|UI")
	FText bossName;

	/** 보스 HP 바 줄 수 (예: 45). 전체 HP 를 이 개수로 나눠 한 줄씩 깎인다 */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|UI", meta = (ClampMin = 1))
	int32 hpLineCount = 10;
};
