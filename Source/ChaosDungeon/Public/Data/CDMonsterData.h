#pragma once

#include "Engine/DataAsset.h"
#include "Core/CDTypes.h"
#include "CDMonsterData.generated.h"

class ACDMonsterBase;
class UCDSkillData;

/** 드랍 항목 */
USTRUCT(BlueprintType)
struct CHAOSDUNGEON_API FCDDropEntry
{
	GENERATED_BODY()

	/** 드랍할 액터 (보통 ACDPickupOrb BP). 몬스터는 구체 클래스를 모른다 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> actorClass;

	/** 드랍 확률 (0 ~ 1) */
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0, ClampMax = 1))
	float chance = 0.1f;
};

/**
 * 몬스터 1종 = 에셋 1개. 일반(근접/원거리)/정예/보스 모두 같은 클래스 + 다른 데이터.
 * 경직/넉백 면역은 grade 로부터 자동 결정된다. (ACDMonsterBase::GetImmunityTags)
 */
UCLASS(BlueprintType)
class CHAOSDUNGEON_API UCDMonsterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 메시/애님BP/AIControllerClass 가 세팅된 ACDMonsterBase 블루프린트 */
	UPROPERTY(EditDefaultsOnly, Category = "Monster")
	TSubclassOf<ACDMonsterBase> monsterClass;

	/** 등급 */
	UPROPERTY(EditDefaultsOnly, Category = "Monster")
	ECDMonsterGrade grade = ECDMonsterGrade::Normal;

	/** 기본 스탯 (난이도 배율 적용 전) */
	UPROPERTY(EditDefaultsOnly, Category = "Monster")
	FCDBaseStats stats;

	/** 처치 게이지 기여도. 보스 스테이지는 보스 기여도 = 스테이지 목표치로 두면 "보스 처치 = 클리어" */
	UPROPERTY(EditDefaultsOnly, Category = "Monster")
	float contribution = 1.f;

	/** 이 거리 안이면 공격 시도 */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float attackRange = 200.f;

	/** 앞쪽일수록 우선 시도 (정예의 특수 공격은 앞에 두고 긴 쿨다운을 준다) */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TArray<TObjectPtr<UCDSkillData>> attackSkills;

	/** 드랍 목록 */
	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	TArray<FCDDropEntry> drops;
};
