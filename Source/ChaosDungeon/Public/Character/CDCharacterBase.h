#pragma once

#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "CDCharacterBase.generated.h"

class UAnimMontage;
class UCDAbilitySystemComponent;
class UCDAttributeSet;
class UCDHitReactAbility;
struct FCDBaseStats;

/**
 * 플레이어/몬스터 공통 베이스
 * - GAS 컴포넌트 소유 + 초기화
 * - 피해/사망을 메시지로 발행 (Combat.Damage, Combat.Death). 누가 듣는지는 모른다.
 * - 무엇을 "할지"는 어빌리티, 누가 "시킬지"는 Controller 가 결정한다.
 */
UCLASS(Abstract)
class CHAOSDUNGEON_API ACDCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	/** GAS 컴포넌트 생성, 이동 설정 */
	ACDCharacterBase();

	// IAbilitySystemInterface::GetAbilitySystemComponent()
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 프로젝트 ASC */
	FORCEINLINE UCDAbilitySystemComponent* GetCDAbilitySystemComponent() const
	{
		return abilitySystem;
	}

	/** 속성 (읽기 전용) */
	FORCEINLINE const UCDAttributeSet* GetAttributes() const
	{
		return attributes;
	}

	/** 피격 몽타주 */
	FORCEINLINE UAnimMontage* GetHitReactMontage() const
	{
		return hitReactMontage;
	}

	/** 스킬 조준 위치. 플레이어 = 커서, 몬스터 = 추적 대상 */
	virtual FVector GetAimLocation() const;

	/** 적대 관계인가 (죽은 대상은 false) */
	bool IsHostileTo(const AActor* other) const;

	/** 사망 상태인가 */
	bool IsDead() const;

	/** 일정 시간 무적 */
	void GrantInvincibility(float duration);

	/** 사망 상태 해제 + 체력 회복. 플레이어 제자리 부활과 몬스터 풀 재사용이 함께 쓴다 */
	void Revive(float invincibleTime);

	/** 디버그용 함수 -> BP 테스트 가능하도록 BlueprintCallable로 선언 */
	UFUNCTION(BlueprintCallable)
	virtual void DebugOnHitFunction(const AActor* attacker);

protected:
	/** 스탯 적용 + 공통 어빌리티 부여. 재호출 가능 (풀링 재사용) */
	void InitializeAbilitySystem(const FCDBaseStats& stats, float healthMultiplier = 1.f, float attackMultiplier = 1.f);

	/** 처치 게이지 기여도 (Combat.Death 메시지에 실림) */
	virtual float GetKillContribution() const;

	/** 피해를 받음 → Combat.Damage 발행 */
	virtual void HandleDamageTaken(AActor* damageInstigator, float damage, bool bCritical);

	/** 체력 0 → 사망 처리 + Combat.Death 발행 */
	virtual void HandleDeath();

protected:
	/** 어빌리티 시스템 */
	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UCDAbilitySystemComponent> abilitySystem;

	/** 속성 */
	UPROPERTY()
	TObjectPtr<UCDAttributeSet> attributes;

	/** 피격 어빌리티 클래스 (기본값 UCDHitReactAbility) */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UCDHitReactAbility> hitReactAbilityClass;

	/** 피격 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> hitReactMontage;

	/** 사망 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> deathMontage;

private:
	/** 최초 1회 초기화 여부 */
	bool bAbilitySystemInitialized = false;
};
