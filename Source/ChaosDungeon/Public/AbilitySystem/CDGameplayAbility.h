#pragma once

#include "Abilities/GameplayAbility.h"
#include "CDGameplayAbility.generated.h"

class ACDCharacterBase;

/**
 * 프로젝트 공용 어빌리티 베이스
 * - 쿨다운: 자식은 GetCooldownDuration / GetCooldownTag 만 알려주면 된다. (쿨다운 GE 에셋을 스킬마다 만들지 않음)
 * - 공용 동작: 조준 방향 회전, 돌진/회피 이동
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API UCDGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/** 인스턴싱/쿨다운 GE/공통 차단 태그 설정 */
	UCDGameplayAbility();

	// UGameplayAbility::GetCooldownTags()
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	// UGameplayAbility::ApplyCooldown()
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo* actorInfo,
		const FGameplayAbilityActivationInfo activationInfo) const override;

protected:
	/** 쿨다운 시간 (0 이면 쿨다운 없음) */
	virtual float GetCooldownDuration() const;

	/** 쿨다운 태그 (어빌리티마다 고유) */
	virtual FGameplayTag GetCooldownTag() const;

	/** 소유 캐릭터 */
	ACDCharacterBase* GetCDCharacter() const;

	/** 이동을 멈추고 location 을 바라본다 (시전 시작 공용 처리) */
	void BeginCastTowards(const FVector& location) const;

	/** 돌진/회피 공용 이동. 어빌리티가 끝나면 이동도 끝난다 */
	void StartDash(const FVector& direction, float distance, float duration);

protected:
	/* 쿨타임 태크 - 10.01 Jun6 Ability별 쿨타임 변수를 갖도록 선언*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	FGameplayTag cooldownTag;

	/* 쿨타임 태크 - 10.01 Jun6 기존 가상 함수 상속에서 정의한 것을 변수로 변경*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	float cooldownDuration = 0.0f;

private:
	/** GetCooldownTags 반환용 캐시 */
	mutable FGameplayTagContainer cooldownTagsCache;
};
