// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CDSkillAbility.h"
#include "CDMonsterSkillAbility.generated.h"

class ACDReticleActor;

/** 몬스터 스킬의 시전, 타격 판정 및 이벤트 처리 */
UCLASS()
class CHAOSDUNGEON_API UCDMonsterSkillAbility : public UCDSkillAbility
{
	GENERATED_BODY()

public:
	UCDMonsterSkillAbility();

	/** 몬스터 스킬 시전 및 이벤트 대기, 몽타주 재생 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData) override;

	/** 몽타주 종료/취소 몬스터 상태 정리 */
	virtual void EndAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
						

protected:
	/** 타격 노티파이 시 몬스터 스킬의 타격 판정 */
	virtual void ApplyHit() override;

	/** 몬스터 스킬의 몽타주 및 VFX 이벤트 처리 */
	virtual void OnEventReceived(FGameplayEventData payload) override;			

	/** 현재 데이터의 판정 기준점 */
	FVector GetSkillOrigin() const;

	virtual void ShowReticle();

	virtual void ClearReticle();

	// 투사체 발사. 보스 특수 패턴은 이 함수 재정의 할 것
	virtual void FireProjectile();

private:
	void SetUntargetable(bool bNewUntargetable);

	bool bOwnsUntargetableTag = false;

protected:
	/** 플레이어의 마우스 조준 Task 없이 AI가 확정한 위치에 표시 */
	UPROPERTY(EditDefaultsOnly, Category = "Reticle")
	TSubclassOf<ACDReticleActor> reticleClass;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ACDReticleActor>> spawnedReticle;
};
