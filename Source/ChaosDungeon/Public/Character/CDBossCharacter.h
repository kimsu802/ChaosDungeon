// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CDMonsterBase.h"
#include <Data/CDBossData.h>
#include "GameplayAbilitySpecHandle.h"
#include "CDBossCharacter.generated.h"

class UCDBossPhaseComponent;

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API ACDBossCharacter : public ACDMonsterBase
{
	GENERATED_BODY()
	
public:
	ACDBossCharacter();

	virtual void BeginPlay() override;

	/** 몬스터 데이터 */
	FORCEINLINE const UCDBossData* GetBossData() const
	{
		return bossData;
	}

	UCDBossPhaseComponent* GetPhaseComponent() const
	{
		return phaseComponent;
	}

	/** 현재 페이즈에서 AI가 선택할 수 있는 공격 목록 */
	FORCEINLINE const TArray<TObjectPtr<UCDSkillData>>& GetActiveAttackSkills() const
	{
		return activeAttackSkills;
	}

	/** 기존 공격을 유지하고 2페이즈 전용 스킬을 추가한다 */
	void EnterPhase2();

protected:
	/** 풀에서 꺼내 데이터/난이도로 초기화 */
	virtual void ActivateFromPool(const UCDMonsterData* data, const FCDDifficultySettings& difficulty, const FVector& location) override;

	/** 사망 연출 후 자동 호출. onReleased 로 스폰 매니저에 반환 */
	virtual void DeactivateToPool() override;

	/** 체력 0 → 사망 처리 + Combat.Death 발행 */
	virtual void HandleDeath() override;


	virtual void HandleDamageTaken(AActor* damageInstigator, float damage, bool bCritical) override;

private:
	/** 추가로 부여한 스킬을 제거하고 기본 공격 목록을 복구한다 */
	void ResetPhaseSkills();

protected:
	/* 맵에 배치한 보스에게 적용할 데이터 */
	UPROPERTY(EditInstanceOnly, Category = "BossInitialization", meta = (DisplayName = "Placed Boss Data"))
	TObjectPtr<UCDMonsterData> placedBossData = nullptr;

	/* 현재 몬스터 데이터 */
	UPROPERTY(Transient)
	TObjectPtr<const UCDBossData> bossData;

	/* 페이즈 관리 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Boss)
	TObjectPtr<UCDBossPhaseComponent> phaseComponent;

private:
	/** 원본 데이터와 분리된 개체별 공격 목록 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCDSkillData>> activeAttackSkills;

	/** 풀 재사용 시 제거할 2페이즈 스킬 핸들 */
	TArray<FGameplayAbilitySpecHandle> phase2SkillHandles;
};
