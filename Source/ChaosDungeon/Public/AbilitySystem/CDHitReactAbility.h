#pragma once

#include "AbilitySystem/CDGameplayAbility.h"
#include "CDHitReactAbility.generated.h"

/**
 * 피격 경직/넉백. Event.Hit 이벤트로 자동 발동된다. (플레이어/몬스터 공용)
 * - State.Immune.Stagger 가 있으면 발동 자체가 막힘 (보스)
 * - State.Immune.Knockback 이 있으면 넉백만 생략 (정예)
 * - 플레이어가 경직되는 공격인지는 공격 스킬 데이터의 bCausesHitReact 가 결정
 */
UCLASS()
class CHAOSDUNGEON_API UCDHitReactAbility : public UCDGameplayAbility
{
	GENERATED_BODY()

public:
	/** 트리거/면역 태그 설정 */
	UCDHitReactAbility();

	// UGameplayAbility::ActivateAbility()
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData) override;

private:
	/** 경직 시간 경과 → 종료 */
	UFUNCTION()
	void OnStaggerFinished();

protected:
	/** 경직 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "HitReact")
	float staggerDuration = 0.4f;
};
