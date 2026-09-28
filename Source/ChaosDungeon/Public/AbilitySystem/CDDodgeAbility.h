#pragma once

#include "AbilitySystem/CDGameplayAbility.h"
#include "CDDodgeAbility.generated.h"

class UAnimMontage;

/**
 * 회피 (Space): 커서 방향 이동 + 짧은 무적 + 쿨다운. 모든 스킬(선딜/후딜)을 캔슬한다.
 * 직업별 몽타주/수치는 BP 자식에서 설정
 */
UCLASS()
class CHAOSDUNGEON_API UCDDodgeAbility : public UCDGameplayAbility
{
	GENERATED_BODY()

public:
	/** 회피 태그/캔슬 규칙 설정 */
	UCDDodgeAbility();

	// UGameplayAbility::ActivateAbility()
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo, const FGameplayEventData* triggerEventData) override;

protected:
	// UCDGameplayAbility::GetCooldownDuration()
	virtual float GetCooldownDuration() const override;

	// UCDGameplayAbility::GetCooldownTag()
	virtual FGameplayTag GetCooldownTag() const override;

private:
	/** 이동 시간 경과 → 종료 */
	UFUNCTION()
	void OnDodgeFinished();

protected:
	/** 회피 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> montage;

	/** 이동 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float distance = 500.f;

	/** 이동 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float duration = 0.25f;

	/** 무적 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float invincibleTime = 0.3f;

	/** 쿨다운 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float cooldown = 4.f;
};
